#include "LSH.hpp"

using namespace std;

extern LSH *Lsh; /* LSH Object */

void Print_values() {
    cout << "L: " << Lsh->get_L() << endl << "k: " << Lsh->get_k() << endl;
    cout << "dimensions: " << dimension << endl << "number of items: " << n << endl;

    /* Print vector v */
    // for (auto vec: v) {
    //     cout << "vector v: ";
    //     for (auto i: vec)
    //         cout << i << ", ";
    //     cout << endl;
    // }

    /* Print vector t */
    cout << "vector t: ";
    for (auto vec: t)
        cout << vec << ", ";  
    cout << endl;
}

long long int mod(long long int value, long long int Mod) {
    if ((value % Mod) < 0) 
        return (unsigned int) (value % Mod + Mod);
    else
        return (unsigned int) (value % Mod);
}

void Initialize_Hash_parameters() {
    dimension = dim_data();
    n = num_of_points();
}

double Normal_distribution() {
    // Inspired from https://en.cppreference.com/w/cpp/numeric/random/normal_distribution
    random_device rd{};
    mt19937 gen{rd()};
 
    normal_distribution<float> d{0, 1};
 
    map<int, int> hist{};
    return round(d(gen));
}

void Euclidean_Hash_Function(int L, int k) {
    // Initialize the vectors used for hashing
    v.resize(k, vector<double>(dimension));

    for(int i=0; i < k; i++) {
        v[i].clear();

		for(int j=0; j < dimension; j++){
			v[i].push_back(Normal_distribution());
		}
    }

    // Initialize w (change it to your liking)
    w = 400;

    srand(time(0));

    // Initialize vector t
    // Inspired from https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
    t.clear();

	random_device generator;
	uniform_real_distribution<float> dis(0.0, (float) w);

	for(int i=0; i<k; i++){ 	// For every hash function
		float random = dis(generator);
		t.push_back(random);
	}

    // Every hash function has a vector length k (otherwise known as r)
    Hash_Functions.resize(L, vector<int> (k));

    for(int i = 0; i < L; i++){
		Hash_Functions[i].clear();

        int j=0;

        do{
			int r_value = rand() % k;

            // auto itA = Hash_Functions[i].begin();

            // while(itA != Hash_Functions[i].end()) {
            //     if (r_value == itA[0]) break;
            //     if (itA != Hash_Functions[i].end()) ++itA;
            //     else {
            //         Hash_Functions[i].push_back(r_value);
            //         j++;
            //         break;
            //     }
            // }
            Hash_Functions[i].push_back(r_value);
            j++;
		}while(j < k);
	}
}

void Calculate_Hash_Value(int L, int k, vector<int> item) {
    for (int g = 0; g < L; g++) {
        long long int hash_value = 0;
        long int ID = -1;
        for (int h = 0; h < k; h++) {
            int sum = 0;
            /* The inner product is calculated by multiplying all the coordinates of 2 vectors and summing them*/
            for (int dim = 1; dim < dimension; dim++) {
                sum += item[dim] * v[h][dim];
            }

            sum += t[h];
            sum = floor(sum / (double) w);
            hash_value += sum * Hash_Functions[g][h];
            hash_value = mod(hash_value, M);
        }

        ID = hash_value;

        hash_value = mod(hash_value, n/4);
        // cout << "for table " << g << " hash value is " << hash_value << endl;
    }
}

long long int euclidean_dis(vector<int> vec1, vector<int> vec2) {
    long long int dist=0;

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

	return dist;
}

LSH::LSH(string input, string query, string output, int L_,int N_,int k_,int R_)
        :input_file(input), query_file(query), output_file(output), L(L_), N(N_), k(k_), R(R_)
    {}


// int* LSH::get_modulars()
// {
//     return modulars;
// }

// int** LSH::get_s_i()
// {
//     return s_i;
// }

// double* LSH::get_tTrue()
// {
//     return tTrue;
// }

// int** LSH::get_True_Distances()
// {
//     return True_Distances;
// }

// int** LSH::get_PointsArray()
// {
//     return points_array;
// }

// int** LSH::get_QueriesArray()
// {
//     return queries_array;
// }

// Bucket*** LSH::get_HashTables()
// {
//     return hashtables;
// }