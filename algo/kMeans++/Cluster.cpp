#include "Cluster.hpp"

using namespace std;
using std::chrono::high_resolution_clock;
using std::chrono::duration;

extern Cluster *cluster;

Cluster::Cluster(string input, string config, string out, bool comp, string method)
                :input_file(input), config_file(config), output_file(out), complete(comp), Method(method) {
                    
                    read_config(config);
                    this->num_of_Items = num_of_points();
                }

void Cluster::read_config(string config_file) {
    ifstream Config_File(config_file);

    if (Config_File.is_open()) {
        string Line;
        while(getline(Config_File, Line)) {
            stringstream s;
            s << Line;
            string name;
            s >> name;
            string var;
            s << Line;
            s >> var;
            name.pop_back();
            if (name == "number_of_clusters") this->number_of_clusters = stoi(var);
            else if (name == "number_of_vector_hash_tables") this->L = stoi(var);
            else if (name == "number_of_vector_hash_functions") this->k = stoi(var);
            else if (name == "max_number_M_hypercube") this->max_number_M_hypercube = stoi(var);
            else if (name == "number_of_hypercube_dimensions") this->number_of_hypercube_dimensions = stoi(var);
            else if (name == "number_of_probes") this->number_of_probes = stoi(var);
        }
        
    }
}

void Cluster::kMeanspp_Initialization() {
    int t = 1, input_items = this->num_of_Items;

    auto begin = high_resolution_clock::now();

    // Pick a random input index
    srand(time(0));
    random_device generator;
	uniform_int_distribution<int> dis(0, input_items);

	int random_index = dis(generator);

    this->centroids.push_back(random_index);

    long double P = 0.0;
    long int next_max_D = -1;

    while(t != this->number_of_clusters) {
        long double sum_min_dists = 0.0;
        long int previous_max_D = next_max_D;
        next_max_D = -1;
        vector<pair<int, long double>> prob;
        for (int point = 0; point < input_items; point++) {
            if (none_of(this->centroids.begin(), this->centroids.end(), [point](int centroid) { return point == centroid; })) {
                long double dis = (long double) NUM;
                int potential_centroid = -1;
                for (auto centroid: this->centroids) {
                    long double point_dist;
                    if (P == 0.0) point_dist = euclidean_dis(this->data[point], this->data[centroid]);
                    else {
                        random_device generator;
                        uniform_real_distribution<float> distance(0.0, P);

                        point_dist = distance(generator);
                        point_dist /= previous_max_D;
                    }

                    if (point_dist < dis) {
                        dis = point_dist;
                        potential_centroid = point;
                    }
                }

                if (dis > next_max_D) next_max_D = dis;

                sum_min_dists += dis;

                prob.push_back(make_pair(potential_centroid, dis));
            }
        }

        P = sum_min_dists;

        long double highest_prob = -1;
        int next_centroid = -1;
        for (auto item: prob) {
            long double prob = item.second/P;
            if (prob > highest_prob) {
                highest_prob = prob;
                next_centroid = item.first;
            }
        }

        this->centroids.push_back(next_centroid);

        t++;
    }

    auto end = high_resolution_clock::now();
    duration<double, std::milli> time = end - begin;

    cout << "kMeans++ TIME: " << time.count() << endl;
}

vector<int> Cluster::Calculate_Mean(vector<int> near_points) {
    long int T = near_points.size();
    int size = this->data[0].size();
    vector<int> centroid(size, 0);
    for (auto point: near_points) {
        for(int i = 1; i < size; i++) {
            // cout << sum[i] << ", ";
            centroid[i] += this->data[point][i];
            // cout << sum[i] << ", " << (long double) this->data[point][i] << endl;
        }
    }

    for (int i = 1; i < size; i++) {
        centroid[i] = centroid[i]/T;
    }

    return centroid;
}

// This function compares all new clusters to their previous
// if the clusters have not changed much then return false else return true
bool Cluster::Compare(vector<vector<int>> previous_clusters) {
    for (int centroid = 0; centroid < number_of_clusters; centroid++) {
        double sum_of_diff_points = 0.0;
        int size = previous_clusters[centroid].size();
        if (size == 0) return true;
        for (int point = 1; point < size; point++) {
            if (Lloyd[centroid].first[point] != previous_clusters[centroid][point]) {
                sum_of_diff_points++;
            }
        }

        // If 10% of the cluster has changed return true
        double percentage = sum_of_diff_points/(double) (size - 1);
        if (percentage >= 0.01) return true;
    }

    return false;
}

void Cluster::Lloyd_method() {

    vector<int> empty_vec;
    empty_vec.clear();
    for (auto centroid: centroids) {
        Lloyd.push_back(make_pair(this->data[centroid], empty_vec));
    }

    centroids.clear();

    auto begin = high_resolution_clock::now();

    vector<vector<int>> previous_clusters(number_of_clusters, empty_vec);
    // previous_clusters.clear();
    // Do this until there is almost no difference to the centroids
    while (Compare(previous_clusters)) {
        // Assign each point to its centroid
        for (int centroid = 0; centroid < number_of_clusters; centroid++) {
            Lloyd[centroid].second.clear();
        }
        for (int point = 0; point < num_of_Items; point++) {
            long double dis = (long double) NUM;
            int point_centroid = -1;
            if (none_of(this->centroids.begin(), this->centroids.end(), [point](int centroid) { return point == centroid; })) {
                for (int centroid = 0; centroid < number_of_clusters; centroid++) {
                    long double point_dist = euclidean_dis(this->data[point], Lloyd[centroid].first);
                    if (point_dist < dis) {
                        dis = point_dist;
                        point_centroid = centroid;
                    }
                }

                Lloyd[point_centroid].second.push_back(point);
            }
        }

        // Store previous clusters before Updating them
        // Update centroids
        for (int centroid = 0; centroid < number_of_clusters; centroid++) {
            previous_clusters[centroid] = Lloyd[centroid].first;
            Lloyd[centroid].first = Calculate_Mean(Lloyd[centroid].second);
        }
    }

    auto end = high_resolution_clock::now();
    Cluster_time = end - begin;
}

void Cluster::Silhouette() {
    vector<long double> sil(number_of_clusters, 0);
    auto begin = high_resolution_clock::now();

    for (int cluster = 0; cluster < number_of_clusters; cluster++) {
        int size_of_cluster = Lloyd[cluster].second.size();
        for (auto i: Lloyd[cluster].second) {
            // Calculate average distance
            long double a = 0.0;
            for (auto j: Lloyd[cluster].second) {
                if (i != j) {
                    a += euclidean_dis(this->data[i], this->data[j]);
                }
            }

            a /= (size_of_cluster-1); // average distance of i to objects in same cluster

            // Find Second closest centroid
            long double dist = NUM;
            int sec_cluster = -1;
            for (int second_cluster = 0; second_cluster < number_of_clusters; second_cluster++) {
                if (cluster != second_cluster) {
                    long double point_dist = euclidean_dis(this->data[i], Lloyd[second_cluster].first);
                    if (dist > point_dist) {
                        sec_cluster = second_cluster;
                        dist = point_dist;
                    }
                }
            }

            // Calculate average distance of second cluster
            long double b = 0.0;
            for (auto j: Lloyd[sec_cluster].second) {
                if (i != j) {
                    b += euclidean_dis(this->data[i], this->data[j]);
                }
            }

            b /= (Lloyd[sec_cluster].second.size()-1);

            // Calculate Silhouette
            sil[cluster] += (b - a) / (b > a ? b : a);
            // cout << sil[i] << ", " << a << ", " << b << endl;
        }

        sil[cluster] /= size_of_cluster;
        // cout << cluster << ": " << sil[cluster] << endl;
    }

    this->s = sil;

    auto end = high_resolution_clock::now();
    duration<double, std::milli> time = end - begin;
    
    cout << "Silhouette TIME: " << time.count() << endl;
}

void Cluster::print() {
    cout << "number_of_clusters: " << number_of_clusters << endl;
    cout << "number_of_vector_hash_tables: " << L << endl;
    cout << "number_of_vector_hash_functions: " << k << endl;
    cout << "max_number_M_hypercube: " << max_number_M_hypercube << endl;
    cout << "number_of_hypercube_dimensions: " << number_of_hypercube_dimensions << endl;
    cout << "number_of_probes: " << number_of_probes << endl;
}

void Cluster::output() {
    ofstream Output;
    Output.open (this->output_file, ofstream::out | ofstream::trunc);
    Output << "Algorithm: ";
    if (this->Method == "Classic" || this->Method == "Lloyd") Output << "Lloyds";
    else if (this->Method == "LSH") Output << "Range Search LSH";
    else if (this->Method == "Hypercube") Output << "Range Search Hypercube";
    Output << endl;

    int counter = 1;
    for (auto centroid: this->Lloyd) {
        bool first = true;
        Output << "CLUSTER-" << counter << " {size: " << centroid.second.size() << ", centroid: ";
        for (auto points: centroid.first) {
            if (first) {
                first = false;
                continue;
            }
            Output << points << ", ";
        }
        if (this->complete) {
            Output << "Items: ";
            for (auto points: centroid.second) {
                Output << points << ", ";
            }
        }
        Output << "}" << endl;
        counter++;
    }

    Output << "cluster_time: " << this->Cluster_time.count() << endl;
    Output << "Silhouette: [";

    long double stotal = 0.0;
    for (auto cluster: this->s) {
        Output << cluster << ",";
        stotal += cluster;
    }

    Output << stotal/this->number_of_clusters << "]" << endl;

    Output.close();
}

// Calculate Euclidean Distance
long double euclidean_dis(vector<int> vec1, vector<int> vec2) {
    long double dist=0.0;

    auto itA = vec1.begin();
    auto itB = vec2.begin();
    ++itA;
    ++itB;

    while(itA != vec1.end() || itB != vec2.end())
    {
        dist = dist + (itA[0]-itB[0])*(itA[0]-itB[0]);
        if(itA != vec1.end()) {
            ++itA;
        }
        if(itB != vec2.end()) {
            ++itB;
        }
    }

	return sqrt(dist);
}