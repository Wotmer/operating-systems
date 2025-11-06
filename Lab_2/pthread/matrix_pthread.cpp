#include <iostream>
#include <vector>
#include <pthread.h>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>
#include <tuple>

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

class MatrixMultiplierPthread {
private:
    const std::vector<std::vector<int>>& A;
    const std::vector<std::vector<int>>& B;
    std::vector<std::vector<int>>& C;
    int block_size;
    int N;

    struct ThreadData {
        MatrixMultiplierPthread* multiplier;
        int start_row, end_row;
        int start_col, end_col;
    };

public:
    MatrixMultiplierPthread(const std::vector<std::vector<int>>& A,
                          const std::vector<std::vector<int>>& B,
                          std::vector<std::vector<int>>& C,
                          int block_size)
        : A(A), B(B), C(C), block_size(block_size), N(A.size()) {}

    void multiplyBlock(int start_row, int end_row, int start_col, int end_col) {
        for (int i = start_row; i < end_row; ++i) {
            for (int j = start_col; j < end_col; ++j) {
                C[i][j] = 0;
                for (int k = 0; k < N; ++k) {
                    C[i][j] += A[i][k] * B[k][j];
                }
            }
        }
    }

    static void* multiplyBlockWrapper(void* arg) {
        ThreadData* data = static_cast<ThreadData*>(arg);
        data->multiplier->multiplyBlock(data->start_row, data->end_row,
                                      data->start_col, data->end_col);
        return nullptr;
    }

    void parallelMultiply() {
        int num_blocks = (N + block_size - 1) / block_size;
        std::vector<pthread_t> threads;
        std::vector<ThreadData> thread_data;

        for (int block_i = 0; block_i < num_blocks; ++block_i) {
            for (int block_j = 0; block_j < num_blocks; ++block_j) {
                int start_row = block_i * block_size;
                int end_row = std::min(start_row + block_size, N);
                int start_col = block_j * block_size;
                int end_col = std::min(start_col + block_size, N);

                ThreadData data;
                data.multiplier = this;
                data.start_row = start_row;
                data.end_row = end_row;
                data.start_col = start_col;
                data.end_col = end_col;
                thread_data.push_back(data);
            }
        }

        for (size_t i = 0; i < thread_data.size(); ++i) {
            pthread_t thread;
            if (pthread_create(&thread, NULL, multiplyBlockWrapper, &thread_data[i]) == 0) {
                threads.push_back(thread);
            }
        }

        for (pthread_t thread : threads) {
            pthread_join(thread, NULL);
        }
    }
};

bool compareByTime(const std::tuple<int, int, long long, long long, double>& a,
                  const std::tuple<int, int, long long, long long, double>& b) {
    return std::get<2>(a) < std::get<2>(b);
}

void runPthreadVersion() {
    std::cout << "=== POSIX pthread Version ===" << std::endl;
    std::cout << "Matrix size: 100x100" << std::endl;
    std::cout << std::string(90, '-') << std::endl;
    std::cout << std::setw(10) << "Block Size"
              << std::setw(15) << "Blocks"
              << std::setw(15) << "Threads"
              << std::setw(20) << "Parallel Time (μs)"
              << std::setw(20) << "Sequential Time (μs)"
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

    for (int block_size : {100, 50, 25, 20, 10, 5, 2, 1}) {
        if (block_size > N) continue;

        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                C_par[i][j] = 0;
            }
        }

        MatrixMultiplierPthread multiplier(A, B, C_par, block_size);

        auto start_par = std::chrono::high_resolution_clock::now();
        multiplier.parallelMultiply();
        auto end_par = std::chrono::high_resolution_clock::now();
        auto duration_par = std::chrono::duration_cast<std::chrono::microseconds>(end_par - start_par);

        int num_blocks = (N + block_size - 1) / block_size;
        int num_threads = num_blocks * num_blocks;
        double speedup = static_cast<double>(sequential_time) / duration_par.count();

        results.push_back(std::make_tuple(block_size, num_threads, duration_par.count(), sequential_time, speedup));

        std::cout << std::setw(10) << block_size
                  << std::setw(10) << num_blocks << "x" << std::setw(4) << num_blocks
                  << std::setw(15) << num_threads
                  << std::setw(20) << duration_par.count()
                  << std::setw(20) << sequential_time
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup << std::endl;
    }

    std::cout << std::string(90, '-') << std::endl;

    auto best_result = *std::min_element(results.begin(), results.end(), compareByTime);

    std::cout << "Best performance: Block size " << std::get<0>(best_result)
              << " (Parallel time: " << std::get<2>(best_result) << " μs, Sequential time: "
              << std::get<3>(best_result) << " μs, Speedup: "
              << std::fixed << std::setprecision(2) << std::get<4>(best_result) << "x)" << std::endl;
    std::cout << std::endl;
}

int main() {
    runPthreadVersion();
    return 0;
}