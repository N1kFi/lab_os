#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <semaphore.h>
#include <chrono>
#include <random>
#include <mutex>
#include <iomanip>
#include <fstream>

using namespace std;

struct Point {
    double x, y;
    int cluster;
};

struct Cluster {
    double x, y;
};

vector<Point> points;
vector<Cluster> clusters;
int k;
int maxThreads;
bool changed = true;

sem_t thread_sem;
pthread_mutex_t mutex_sum;

double distance(const Point &a, const Cluster &b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

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

    if (local_changed) {
        pthread_mutex_lock(&mutex_sum);
        changed = true;
        pthread_mutex_unlock(&mutex_sum);
    }

    sem_post(&thread_sem);
    return nullptr;
}

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

// Функция для вывода результатов
void print_results() {
    cout << fixed << setprecision(2);
    
    // 1. Вывод центров кластеров
    cout << "\n═══════════════════════════════════════════════════\n";
    cout << "               РЕЗУЛЬТАТЫ КЛАСТЕРИЗАЦИИ\n";
    cout << "═══════════════════════════════════════════════════\n";
    
    cout << "\nЦЕНТРЫ КЛАСТЕРОВ:\n";
    cout << "┌─────────┬────────────┬────────────┐\n";
    cout << "│ Кластер │     X      │     Y      │\n";
    cout << "├─────────┼────────────┼────────────┤\n";
    for (int i = 0; i < k; ++i) {
        cout << "│    " << i << "    │ " 
             << setw(10) << clusters[i].x << " │ " 
             << setw(10) << clusters[i].y << " │\n";
    }
    cout << "└─────────┴────────────┴────────────┘\n";

    // 2. Статистика по кластерам
    vector<int> cluster_sizes(k, 0);
    for (auto &p : points) {
        cluster_sizes[p.cluster]++;
    }
    
    cout << "\nСТАТИСТИКА ПО КЛАСТЕРАМ:\n";
    cout << "┌─────────┬────────────┬──────────────┐\n";
    cout << "│ Кластер │ Количество │  Процент (%) │\n";
    cout << "├─────────┼────────────┼──────────────┤\n";
    for (int i = 0; i < k; ++i) {
        double percentage = (cluster_sizes[i] * 100.0) / points.size();
        cout << "│    " << i << "    │ " 
             << setw(10) << cluster_sizes[i] << " │ " 
             << setw(12) << percentage << " │\n";
    }
    cout << "└─────────┴────────────┴──────────────┘\n";

    // 3. Вывод первых 20 точек
    int points_to_show = min(20, (int)points.size());
    cout << "\nПЕРВЫЕ " << points_to_show << " ТОЧЕК:\n";
    cout << "┌───────┬────────────┬────────────┬─────────┐\n";
    cout << "│ Точка │     X      │     Y      │ Кластер │\n";
    cout << "├───────┼────────────┼────────────┼─────────┤\n";
    
    for (int i = 0; i < points_to_show; ++i) {
        cout << "│ " << setw(5) << i << " │ " 
             << setw(10) << points[i].x << " │ " 
             << setw(10) << points[i].y << " │ " 
             << setw(7) << points[i].cluster << " │\n";
    }
    cout << "└───────┴────────────┴────────────┴─────────┘\n";

    // 4. Сохранение в файл для визуализации
    ofstream outfile("clusters_output.csv");
    if (outfile.is_open()) {
        outfile << "point_id,x,y,cluster\n";
        for (size_t i = 0; i < points.size(); ++i) {
            outfile << i << "," << points[i].x << "," 
                   << points[i].y << "," << points[i].cluster << "\n";
        }
        outfile.close();
    }

    // 5. Сохранение центров кластеров отдельно
    ofstream centers_file("cluster_centers.csv");
    if (centers_file.is_open()) {
        centers_file << "cluster_id,center_x,center_y,points_count\n";
        for (int i = 0; i < k; ++i) {
            centers_file << i << "," << clusters[i].x << "," 
                        << clusters[i].y << "," << cluster_sizes[i] << "\n";
        }
        centers_file.close();
        cout << "✓ Центры кластеров сохранены в 'cluster_centers.csv'\n";
    }

    cout << "\n═══════════════════════════════════════════════════\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Использование: ./lab4 <макс_потоков>\n";
        cout << "Пример: ./lab4 4\n";
        return 1;
    }

    maxThreads = stoi(argv[1]);
    cout << "═══════════════════════════════════════════════════\n";
    cout << "  АЛГОРИТМ K-СРЕДНИХ С " << maxThreads << " ПОТОКАМИ\n";
    cout << "═══════════════════════════════════════════════════\n\n";

    cout << "Введите число кластеров (k): ";
    cin >> k;

    int n;
    cout << "Введите количество точек: ";
    cin >> n;

    if (k <= 0 || n <= 0) {
        cout << "Ошибка: k и n должны быть положительными числами!\n";
        return 1;
    }

    if (k > n) {
        cout << "Предупреждение: кластеров больше, чем точек!\n";
    }

    // Инициализация генератора случайных чисел
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0, 100);

    // Создание случайных точек
    points.resize(n);
    cout << "\nГенерация " << n << " случайных точек...\n";
    for (auto &p : points) {
        p.x = dis(gen);
        p.y = dis(gen);
        p.cluster = rand() % k;  // Начальное случайное присвоение
    }

    // Инициализация центров кластеров
    clusters.resize(k);
    cout << "Инициализация " << k << " центров кластеров...\n";
    for (auto &c : clusters) {
        c.x = dis(gen);
        c.y = dis(gen);
    }

    // Инициализация примитивов синхронизации
    sem_init(&thread_sem, 0, maxThreads);
    pthread_mutex_init(&mutex_sum, nullptr);

    // Запуск алгоритма
    cout << "\nЗапуск алгоритма k-средних...\n";
    cout << "Итерации:\n";
    
    auto start_time = chrono::high_resolution_clock::now();
    int iteration = 0;

    while (changed) {
        changed = false;
        iteration++;

        vector<pthread_t> threads(maxThreads);

        // Запуск потоков для назначения точек
        for (int i = 0; i < maxThreads; ++i) {
            sem_wait(&thread_sem);
            pthread_create(&threads[i], nullptr, assign_points, (void*)(long)i);
        }

        // Ожидание завершения всех потоков
        for (int i = 0; i < maxThreads; ++i) {
            pthread_join(threads[i], nullptr);
        }

        // Обновление центров кластеров
        update_clusters();
        
        // Вывод прогресса
        cout << "  Итерация " << iteration << " завершена\n";
        
        // Предохранитель от бесконечного цикла
        if (iteration > 100) {
            cout << "Предупреждение: достигнут лимит в 100 итераций\n";
            break;
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    // Вывод результатов
    print_results();
    
    cout << "\n═══════════════════════════════════════════════════\n";
    cout << "              СВОДНАЯ ИНФОРМАЦИЯ\n";
    cout << "═══════════════════════════════════════════════════\n";
    cout << "• Количество потоков: " << maxThreads << "\n";
    cout << "• Количество кластеров: " << k << "\n";
    cout << "• Количество точек: " << n << "\n";
    cout << "• Количество итераций: " << iteration << "\n";
    cout << "• Время выполнения: " << duration << " мс\n";
    
    if (duration > 0) {
        double points_per_ms = n / (double)duration;
        cout << "• Скорость обработки: " << fixed << setprecision(2) 
             << points_per_ms << " точек/мс\n";
    }
    
    cout << "═══════════════════════════════════════════════════\n";

    // Очистка ресурсов
    sem_destroy(&thread_sem);
    pthread_mutex_destroy(&mutex_sum);
    
    return 0;
}
