#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>
#include <tuple>
#include "buffered_channel.h"

struct MatrixTask {
    int start_row;
    int end_row;
    int start_col;
    int end_col;
};

std::vector<std::vector<int>> generateRandomMatrix(int N) {
    std::vector<std::vector<int>> matrix(N, std::vector<int>(N));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10);

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            matrix[i][j] = dis(gen);
        }
    }
    return matrix;
}

void sequentialMultiply(const std::vector<std::vector<int>>& A,
                        const std::vector<std::vector<int>>& B,
                        std::vector<std::vector<int>>& C) {
    int N = A.size();
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < N; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

class MatrixMultiplierChannel {
private:
    const std::vector<std::vector<int>>& A;
    const std::vector<std::vector<int>>& B;
    std::vector<std::vector<int>>& C;
    int block_size;
    int N;

    BufferedChannel<MatrixTask>* channel;

public:
    MatrixMultiplierChannel(const std::vector<std::vector<int>>& A,
                            const std::vector<std::vector<int>>& B,
                            std::vector<std::vector<int>>& C,
                            int block_size)
        : A(A), B(B), C(C), block_size(block_size), N(A.size()), channel(nullptr) {}


    void worker() {
        while (true) {
            auto task_pair = channel->Recv();
            if (!task_pair.second) {
                break;
            }

            MatrixTask task = task_pair.first;

            for (int i = task.start_row; i < task.end_row; ++i) {
                for (int j = task.start_col; j < task.end_col; ++j) {
                    int sum = 0;
                    for (int k = 0; k < N; ++k) {
                        sum += A[i][k] * B[k][j];
                    }
                    C[i][j] = sum;
                }
            }
        }
    }

    void parallelMultiply() {
        unsigned int num_workers = std::thread::hardware_concurrency();
        if (num_workers == 0) num_workers = 4;

        int buffer_capacity = std::max(10, (int)num_workers * 2);
        channel = new BufferedChannel<MatrixTask>(buffer_capacity);

        std::vector<std::thread> threads;

        for (unsigned int i = 0; i < num_workers; ++i) {
            threads.emplace_back(&MatrixMultiplierChannel::worker, this);
        }
        int num_blocks = (N + block_size - 1) / block_size;

        for (int block_i = 0; block_i < num_blocks; ++block_i) {
            for (int block_j = 0; block_j < num_blocks; ++block_j) {
                MatrixTask task;
                task.start_row = block_i * block_size;
                task.end_row = std::min(task.start_row + block_size, N);
                task.start_col = block_j * block_size;
                task.end_col = std::min(task.start_col + block_size, N);
                channel->Send(task);
            }
        }
        channel->Close();
        for (auto& thread : threads) {
            thread.join();
        }

        delete channel;
    }
};

bool compareByTime(const std::tuple<int, int, long long, long long, double>& a,
                   const std::tuple<int, int, long long, long long, double>& b) {
    return std::get<2>(a) < std::get<2>(b);
}

void runChannelVersion() {
    std::cout << "=== C++ Channel (Worker Pool) Version ===" << std::endl;
    unsigned int threads_cnt = std::thread::hardware_concurrency();
    std::cout << "Active Worker Threads: " << (threads_cnt ? threads_cnt : 4) << std::endl;
    std::cout << "Matrix size: 100x100" << std::endl;
    std::cout << std::string(90, '-') << std::endl;
    std::cout << std::setw(10) << "Block Size"
              << std::setw(15) << "Blocks(Tasks)"
              << std::setw(15) << "Tasks Count"
              << std::setw(20) << "Parallel Time (us)"
              << std::setw(20) << "Sequential Time (us)"
              << std::setw(15) << "Speedup" << std::endl;
    std::cout << std::string(90, '-') << std::endl;

    int N = 100;
    auto A = generateRandomMatrix(N);
    auto B = generateRandomMatrix(N);
    std::vector<std::vector<int>> C_seq(N, std::vector<int>(N));
    std::vector<std::vector<int>> C_par(N, std::vector<int>(N));
    auto start_seq = std::chrono::high_resolution_clock::now();
    sequentialMultiply(A, B, C_seq);
    auto end_seq = std::chrono::high_resolution_clock::now();
    auto duration_seq = std::chrono::duration_cast<std::chrono::microseconds>(end_seq - start_seq);

    long long sequential_time = duration_seq.count();

    std::vector<std::tuple<int, int, long long, long long, double>> results;

    for (int block_size : {1, 2, 5, 10, 20, 25, 50, 100}) {
        if (block_size > N) continue;
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                C_par[i][j] = 0;
            }
        }

        MatrixMultiplierChannel multiplier(A, B, C_par, block_size);

        auto start_par = std::chrono::high_resolution_clock::now();
        multiplier.parallelMultiply();
        auto end_par = std::chrono::high_resolution_clock::now();
        auto duration_par = std::chrono::duration_cast<std::chrono::microseconds>(end_par - start_par);

        int num_blocks_dim = (N + block_size - 1) / block_size;
        int total_tasks = num_blocks_dim * num_blocks_dim;
        double speedup = static_cast<double>(sequential_time) / duration_par.count();

        results.push_back(std::make_tuple(block_size, total_tasks, duration_par.count(), sequential_time, speedup));

        std::cout << std::setw(10) << block_size
                  << std::setw(10) << num_blocks_dim << "x" << std::setw(4) << num_blocks_dim
                  << std::setw(15) << total_tasks
                  << std::setw(20) << duration_par.count()
                  << std::setw(20) << sequential_time
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup << std::endl;
    }

    std::cout << std::string(90, '-') << std::endl;

    auto best_result = *std::min_element(results.begin(), results.end(), compareByTime);

    std::cout << "Best performance: Block size " << std::get<0>(best_result)
              << " (Parallel time: " << std::get<2>(best_result) << " us, Sequential time: "
              << std::get<3>(best_result) << " us, Speedup: "
              << std::fixed << std::setprecision(2) << std::get<4>(best_result) << "x)" << std::endl;
    std::cout << std::endl;
}

int main() {
    runChannelVersion();
    return 0;
}