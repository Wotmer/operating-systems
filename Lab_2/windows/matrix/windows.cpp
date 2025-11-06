#define NOMINMAX
#include <iostream>
#include <vector>
#include <windows.h>
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
    int N = static_cast<int>(A.size());
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < N; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

class MatrixMultiplierWindows {
private:
    std::vector<std::vector<int>> A;
    std::vector<std::vector<int>> B;
    std::vector<std::vector<int>> C;
    int block_size;
    int N;

    struct ThreadData {
        std::vector<std::vector<int>>* A;
        std::vector<std::vector<int>>* B;
        std::vector<std::vector<int>>* C;
        int start_row, end_row;
        int start_col, end_col;
        int N;
    };

public:
    MatrixMultiplierWindows(const std::vector<std::vector<int>>& A,
        const std::vector<std::vector<int>>& B,
        std::vector<std::vector<int>>& C,
        int block_size)
        : A(A), B(B), C(C), block_size(block_size), N(static_cast<int>(A.size())) {
    }

    static DWORD WINAPI multiplyBlockWrapper(LPVOID lpParam) {
        ThreadData* data = static_cast<ThreadData*>(lpParam);
        for (int i = data->start_row; i < data->end_row; ++i) {
            for (int j = data->start_col; j < data->end_col; ++j) {
                (*data->C)[i][j] = 0;
                for (int k = 0; k < data->N; ++k) {
                    (*data->C)[i][j] += (*data->A)[i][k] * (*data->B)[k][j];
                }
            }
        }
        return 0;
    }

    void parallelMultiply(std::vector<std::vector<int>>& result) {
        this->C = result;
        int num_blocks = (N + block_size - 1) / block_size;
        std::vector<HANDLE> threads;
        std::vector<ThreadData> thread_data;

        for (int block_i = 0; block_i < num_blocks; ++block_i) {
            for (int block_j = 0; block_j < num_blocks; ++block_j) {
                int start_row = block_i * block_size;
                int end_row = std::min(start_row + block_size, N);
                int start_col = block_j * block_size;
                int end_col = std::min(start_col + block_size, N);

                ThreadData data;
                data.A = &this->A;
                data.B = &this->B;
                data.C = &this->C;
                data.start_row = start_row;
                data.end_row = end_row;
                data.start_col = start_col;
                data.end_col = end_col;
                data.N = N;
                thread_data.push_back(data);
            }
        }

        for (size_t i = 0; i < thread_data.size(); ++i) {
            HANDLE thread = CreateThread(NULL, 0, multiplyBlockWrapper, &thread_data[i], 0, NULL);
            if (thread != NULL) {
                threads.push_back(thread);
            }
        }

        WaitForMultipleObjects(static_cast<DWORD>(threads.size()), threads.data(), TRUE, INFINITE);

        for (HANDLE thread : threads) {
            CloseHandle(thread);
        }

        result = this->C;
    }
};

bool compareByTime(const std::tuple<int, int, long long, long long, double>& a,
    const std::tuple<int, int, long long, long long, double>& b) {
    return std::get<2>(a) < std::get<2>(b);
}

void runWindowsVersion() {
    std::cout << "=== Windows API Version ===" << std::endl;
    std::cout << "Matrix size: 50x50" << std::endl;
    std::cout << std::string(90, '-') << std::endl;
    std::cout << std::setw(10) << "Block Size"
        << std::setw(15) << "Blocks"
        << std::setw(15) << "Threads"
        << std::setw(20) << "Parallel Time (μs)"
        << std::setw(20) << "Sequential Time (μs)"
        << std::setw(15) << "Speedup" << std::endl;
    std::cout << std::string(90, '-') << std::endl;

    int N = 50;
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

    for (int block_size : {25, 10, 5}) {
        if (block_size > N) continue;

        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                C_par[i][j] = 0;
            }
        }

        MatrixMultiplierWindows multiplier(A, B, C_par, block_size);

        auto start_par = std::chrono::high_resolution_clock::now();
        multiplier.parallelMultiply(C_par);
        auto end_par = std::chrono::high_resolution_clock::now();
        auto duration_par = std::chrono::duration_cast<std::chrono::microseconds>(end_par - start_par);

        int num_blocks = (N + block_size - 1) / block_size;
        int num_threads = num_blocks * num_blocks;
        double speedup = (duration_par.count() > 0) ? static_cast<double>(sequential_time) / duration_par.count() : 0.0;

        results.push_back(std::make_tuple(block_size, num_threads, duration_par.count(), sequential_time, speedup));

        std::cout << std::setw(10) << block_size
            << std::setw(10) << num_blocks << "x" << std::setw(4) << num_blocks
            << std::setw(15) << num_threads
            << std::setw(20) << duration_par.count()
            << std::setw(20) << sequential_time
            << std::setw(15) << std::fixed << std::setprecision(2) << speedup << std::endl;
    }

    std::cout << std::string(90, '-') << std::endl;

    if (!results.empty()) {
        auto best_result = *std::min_element(results.begin(), results.end(), compareByTime);

        std::cout << "Best performance: Block size " << std::get<0>(best_result)
            << " (Parallel time: " << std::get<2>(best_result) << " μs, Sequential time: "
            << std::get<3>(best_result) << " μs, Speedup: "
            << std::fixed << std::setprecision(2) << std::get<4>(best_result) << "x)" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    runWindowsVersion();
    return 0;
}