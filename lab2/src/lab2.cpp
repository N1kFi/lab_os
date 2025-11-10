#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <semaphore.h>
#include <chrono>
#include <random>
#include <mutex>

using namespace std;

// ======= Структуры данных =======
struct Point {
    double x, y;
    int cluster;
};

struct Cluster {
    double x, y;
};

// ======= Глобальные переменные =======
vector<Point> points;
vector<Cluster> clusters;
int k;                      // число кластеров
int maxThreads;             // максимальное число потоков
bool changed = true;        // флаг изменения кластеров

sem_t thread_sem;           // семафор для ограничения потоков
pthread_mutex_t mutex_sum;  // мьютекс для синхронизации

// ======= Функция расстояния =======
double distance(const Point &a, const Cluster &b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// ======= Поточная функция =======
void* assign_points(void* arg) {
    long id = (long)arg;
    size_t total_points = points.size();
    size_t points_per_thread = total_points / maxThreads;

    size_t start = id * points_per_thread;
    size_t end = (id == maxThreads - 1) ? total_points : start + points_per_thread;

    bool local_changed = false;

    for (size_t i = start; i < end; ++i) {
        double min_dist = 1e9;
        int cluster_index = 0;

        for (int j = 0; j < k; ++j) {
            double dist = distance(points[i], clusters[j]);
            if (dist < min_dist) {
                min_dist = dist;
                cluster_index = j;
            }
        }

        if (points[i].cluster != cluster_index) {
            points[i].cluster = cluster_index;
            local_changed = true;
        }
    }

    // если в этом потоке что-то изменилось — фиксируем
    if (local_changed) {
        pthread_mutex_lock(&mutex_sum);
        changed = true;
        pthread_mutex_unlock(&mutex_sum);
    }

    sem_post(&thread_sem); // освободить слот семафора
    return nullptr;
}

// ======= Пересчёт центров кластеров =======
void update_clusters() {
    vector<double> sum_x(k, 0.0), sum_y(k, 0.0);
    vector<int> count(k, 0);

    for (auto &p : points) {
        sum_x[p.cluster] += p.x;
        sum_y[p.cluster] += p.y;
        count[p.cluster]++;
    }

    for (int i = 0; i < k; ++i) {
        if (count[i] > 0) {
            clusters[i].x = sum_x[i] / count[i];
            clusters[i].y = sum_y[i] / count[i];
        }
    }
}

// ======= Главная функция =======
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Использование: ./lab4 <макс_потоков>\n";
        return 1;
    }

    maxThreads = stoi(argv[1]);
    cout << "Введите число кластеров: ";
    cin >> k;

    int n;
    cout << "Введите количество точек: ";
    cin >> n;

    // случайные точки
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0, 100);

    points.resize(n);
    for (auto &p : points) {
        p.x = dis(gen);
        p.y = dis(gen);
        p.cluster = rand() % k;
    }

    clusters.resize(k);
    for (auto &c : clusters) {
        c.x = dis(gen);
        c.y = dis(gen);
    }

    sem_init(&thread_sem, 0, maxThreads);
    pthread_mutex_init(&mutex_sum, nullptr);

    auto start_time = chrono::high_resolution_clock::now();

    // Основной цикл K-средних
    while (changed) {
        changed = false;

        vector<pthread_t> threads(maxThreads);

        for (int i = 0; i < maxThreads; ++i) {
            sem_wait(&thread_sem);
            pthread_create(&threads[i], nullptr, assign_points, (void*)(long)i);
        }

        for (int i = 0; i < maxThreads; ++i) {
            pthread_join(threads[i], nullptr);
        }

        update_clusters();
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "\nАлгоритм завершён.\nВремя выполнения: " << duration << " мс\n";

    sem_destroy(&thread_sem);
    pthread_mutex_destroy(&mutex_sum);
    return 0;
}
