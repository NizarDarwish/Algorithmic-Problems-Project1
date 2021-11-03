#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <ctime>
#include <utility>
#include <list>
#include <string>
#include <random>
#include<algorithm> // for heap 

typedef std::vector<std::list<int>> hypercube_hashtable;

class Euclidean_H_i {
    private:

        std::vector<std::vector<double>> v; /* Vectors with random normal numbers, used for hashfunction */
        std::vector<double> t; // Vector with random numbers between 0 and w picked uniformly  
		int w ;
		int k;
		int dimlim;
    public:

		double Normal_distribution(){
			// Inspired from https://en.cppreference.com/w/cpp/numeric/random/normal_distribution
			std::random_device rd{};
			std::mt19937 gen{rd()};
			std::normal_distribution<float> d{0, 1};
		
			return round(d(gen));
		}

        Euclidean_H_i(int k_, int dimlim_){
			int dimlim = dimlim_;
			k=k_;
			// Initialize the vectors used for hashing
			v.resize(k, std::vector<double>(dimlim));

			for(int i=0; i < k; i++) {
				v[i].clear();

				for(int j=0; j < dimlim; j++){
					v[i].push_back(Normal_distribution());
				}
			}

			// Initialize w (change it to your liking)
			w = 500;

			srand(time(0));

			// Initialize vector t
			// Inspired from https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
			t.clear();

			std::random_device generator;
			std::uniform_real_distribution<float> dis(0.0, (float) w);

			for(int i=0; i<k; i++){ 	// For every hash function
				float random = dis(generator);
				t.push_back(random);
			}

		}

        ~Euclidean_H_i(){
    		v.clear();
    		t.clear();
		} 
		
		int Specific_Hi(std::vector<int> item,int var) {
			long int sum;
			for (int var2 = 1; var2 < dimlim; var2++) {
        		sum += item[var2] * v[var][var2];
    		}
			sum += t[var];
    		sum = floor(sum /(double) w);
			return sum;
		}
		
        std::vector<std::vector <double>> get_vector_v() { return v; }
        std::vector<double> get_vector_t() { return t; }
};

class Hypercube{
	private:
    
// For every vertice find all neighbors and scan up to probes vertices(sorted by hamming Distance)  with min heap
        typedef struct Neighbor_Vertice{
            int hamming_distance; // Hamming distance with current id
            int id; // id in  hypercube_hashtable 
        
            Neighbor_Vertice(int dist, int id){
                this->hamming_distance = dist;
                this->id = id;
            }
        }Neighbor_vertice;

        //  Compare class based for hamming distance
        struct Compare_Vertices{
            bool operator()(const Neighbor_Vertice& x, const Neighbor_Vertice& y) const{
                return x.hamming_distance < y.hamming_distance;
            }
        };


    int k; // number of hash functions , meaning the size of hypercube
	int threshold; // max elements to be checked during the program
    int buckets_num; // number of buckets in the hash table
    int points_num; // number of points to be hashed
    int w; // a number signifficantly larger thar the radius, affects range search
    int space; // the dimension of the space we are going to have
	int max_probes; // max probes of the hypercube to be checked during search
	int N;


    // Hypercube is a hashtable
	hypercube_hashtable hypercube;
	Euclidean_H_i *H_i_ptr;
	// vector of the vectors is given 
	std::vector<std::vector<int>> data_vectors; 
	std::vector<std::uniform_int_distribution<int> > f_i; // Distributions for fi 
	std::default_random_engine generator; 
    std::vector<std::unordered_map<int, bool>  > Hi_map; //  map f with unique values
	// generator and the random distribution for the mapping
	public:
		int R;
		std::string input_file;
		std::string query_file;
		std::vector<std::vector<int>> query_data;

		Hypercube(std::string input_, std::string query_,int R_,int k_,int threshold_, 
		int points_num_,int N_,int space_,int max_probes_,std::vector<std::vector<int>> data_vectors_) :input_file(input_), query_file(query_),  
		R(R_),k(k_), threshold(threshold_), points_num(points_num_),N(N_),space(space_),max_probes(max_probes_),data_vectors(data_vectors_) {
			
            //measure the time that it takes to initialize the hypercube
            time_t start, finish;
            fprintf(stdout, "Hypercube program with parameters:  k = %d, threshold = %d points_num = %d, space = %d, max_probes = %d,", k, threshold, points_num, space, max_probes);
            
            time(&start);

            // we have a bitstring of k bits, thus the size of the hypercube table will be 2^k
            buckets_num = (int)pow(2, k);

            //Create hash functions for each bucket
            this->H_i_ptr = new Euclidean_H_i(k,space);

			// Set size of H
        	this->Hi_map.reserve(k_);
			this->hypercube.reserve(this->buckets_num);

			for(int i=0;i<this->k;i++){
    			f_i.push_back(uniform_int_distribution<int>(0,1));
				this->Hi_map.push_back(unordered_map<int, bool>());
			}

            // initialize the hypercube hashtable
            //every bucket contains a list of integers
            //each one integers means the id of the point at the data
            // for example : the number 2 means the second point etc ..
            for (int i = 0; i < buckets_num; i++) {
                std::list<int> temp_list;
                hypercube.push_back(temp_list);
            }
			
			bool var = false;
			//insert all of the points into the hypercube
			for (int i = 0; i < points_num; i++) {
				int bucket_id = hypercube_hash(data_vectors.at(i),var);
				this->hypercube[bucket_id].push_back(i);
			}

			// hypercube initialization done. print the time differenct

			time(&finish);
			std::cout << " Hypercube created in " << difftime(finish, start) << " sec" << std::endl;
		}

		~Hypercube(){};

		int hypercube_hash(std::vector<int> vec_point, bool& query){
			// store the string produced
			std::string bucket_string = "";
			int curr_Hi;
			bool curr_Fi;
			std::unordered_map<int, bool>::const_iterator iterh; // Iterate through map
			std::default_random_engine generator;

			// hash k times
			for(int i=0;i<this->k;i++){
                    // hash with the i-th hash fn
					curr_Hi = this->H_i_ptr->Specific_Hi(vec_point,i);
					// Find if H[i] value exists 
        			iterh = this->Hi_map[i].find(curr_Hi);

					// Exists 
        			if(iterh != this->Hi_map[i].end())
            			curr_Fi = iterh->second;
					// Map current H[i] and add it in the map 
        			else{
            			curr_Fi = this->f_i[i](generator);
						if(query == false)
            				this->Hi_map[i].insert(std::pair<int, bool>(curr_Hi, curr_Fi));
        			}
					 bucket_string += std::to_string(curr_Fi);
		    }

			// convert the bitstring into an integer
			return stoi(bucket_string, nullptr, 2);
		}

		// Calculate Euclidean Distance
		long double euclidean_dis(std::vector<int> vec1, std::vector<int> vec2) {
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

	// Function to calculate hamming distance
	int hammingDistance(int n1, int n2)
	{
		int x = n1 ^ n2;
		int setBits = 0;
	
		while (x > 0) {
			setBits += x & 1;
			x >>= 1;
		}
	
		return setBits;
	}
 

	// // Print statistics of sub hash function 
	// void hashFunctionEuclideanHypercube::print(void){

	// 	if(this->k == -1)
	// 		cout << "Invalid euclidean hypercube hash function\n";
	// 	else{
	// 		int i;
			
	// 		cout << "Euclidean hypercube hash function id: " << this->id << "\n";
	// 		cout << "Value of k: " << this->k << "\n";
			
	// 		cout << "Statistics of sub hash functions: \n\n";
			
	// 		for(i = 0; i < this->bucket_num; i++){
	// 			for(j=0;j<this->hypercube[i].size()){

	// 			}
	// 		}
	// 			this->H[i]->print();
	// 	}
	// }
	void RNeighbors(std::vector<int> query, int radius, std::list<int>& neighbors, std::list<double>& neighbors_dists){ 
		int i, cube_id, pos;
		double curr_Dist; // Distance of a point in list
		std::vector<Neighbor_Vertice> Neighbor_Vertices; // Keep all neighbors  
		int neighbors_num = 0; 
		
		// Clear given lists 
		//neighbors.clear();
		//if(neighbors_dists != NULL)
		//	neighbors_dists->clear();

		// Find initial vertice 
		bool var = true;
		cube_id = hypercube_hash(query,var);

		// Find all neighbors of that vertice 
		for(i = 0; i < this->buckets_num; i++){

			// Don't check the same position 
			if(i == cube_id)
				continue;

			Neighbor_Vertices.push_back(Neighbor_Vertice(hammingDistance(cube_id, i), i));
		}

		// Sort Neighbor Vertices 
		make_heap(Neighbor_Vertices.begin(), Neighbor_Vertices.end(), Compare_Vertices());//, Compare_Vertices()
		sort_heap(Neighbor_Vertices.begin(), Neighbor_Vertices.end(), Compare_Vertices());

		//  for(i = 0; i < this->buckets_num; i++){
		//  	std::cout << Neighbor_Vertices[i].id << " ";
		// }
		//std::cout << "probes" << this->max_probes  ;

		// Check probes vertices for neighbors 
		for(i = 0; i < this->max_probes; i++){
	
			// Check initial pos 
			if(i == 0)
				pos = cube_id;
			// Extract minimum position
			else
				pos = Neighbor_Vertices[i-1].id;

			// Empty vertice(bucket) 
			if(this->hypercube[pos].size() == 0){
				//cout << "here " << i << "\n";
				continue;
			}

			// Scan current vertice 
			for(auto iter = this->hypercube[pos].begin(); iter != this->hypercube[pos].end(); iter++){  
				neighbors_num += 1;
				
				// Find current distance 
				curr_Dist = euclidean_dis(data_vectors.at(*iter),query);

				// Keep neighbor 
				if(curr_Dist < radius){
					neighbors.push_back(*iter);
					//if(neighbors_dists != NULL)
						neighbors_dists.push_back(curr_Dist);
				}

				// Found m neighbors 
				if(neighbors_num == this->threshold)
					break;
			} 
			// end for - Found m neighbors 
			if(neighbors_num == this->threshold)
				break;
		} // end for - Probes
	}

// 	/* Find the nearest neighbor of a given point */
// void hypercubeEuclidean::nNeighbor(vector<int> query, int radius, list<int>& neighbors, list<double>* neighbors_dists){
//     int i, initialPos, pos, found = 0, flag = 0;
//     double currDist; // Distance of a point in list
//     double d = -1;
//     list<Item>::iterator iter;
//     list<Item>::iterator iterNearestNeighbor;
//     vector<Neighbor_Vertice> neighborVertices; // Keep all neighbors  
//     int neighbors_num = 0; // Number of neighbors

// 	// Find initial vertice 
// 	cube_id = hypercube_hash(query);

//     // Find all neighbors of that vertice 
// 	for(i = 0; i < this->buckets_num; i++){

// 		// Don't check the same position 
// 		if(i == cube_id)
// 			continue;
// 			Neighbor_Vertices.push_back(Neighbor_Vertice(hammingDistance(cube_id, i), i));
// 	}

//     // Sort Neighbor Vertices 
// 	make_heap(Neighbor_Vertices.begin(), Neighbor_Vertices.end(), Compare_Vertices());

//     // Check probes vertices for neighbors 
// 		for(i = 0; i < this->max_probes; i++){
	
// 			// Check initial pos 
// 			if(i == 0)
// 				pos = cube_id
// 			// Extract minimum position
// 			else
// 				pos = Neighbor_Vertices.front().id;

// 			// Empty vertice 
// 			if(this-> hypercube_hashtable[pos].size() == 0)
// 				continue;

//         // Scan current vertice 
// 		for(auto iter = this-> hypercube_hashtable[pos].begin(); iter != this-> hypercube_hashtable[pos].end(); iter++){  
// 				neighbors_num += 1;
				
// 				// Find current distance 
// 				curr_Dist = euclidean_dis(data_vectors.at(*iter),query);
				
            
//             if (euc_dist < d) {
//                 if (near_items.size() >= N) {
//                     d = euc_dist;
//                     b = index;
//                     if (none_of(near_items.begin(), near_items.end(), [b](pair<int, int> item) { return b == item.second; })) {
//                         near_items.pop_back();
//                         near_items.insert(near_items.begin(), make_pair(d, b));
//                     }
//                 } else {
//                     d = euc_dist;
//                     b = index;
//                     near_items.push_back(make_pair(d, b));
//                     sort(near_items.begin(), near_items.end());
//                 }

//             // Found m neighbors 
//             if(neighbors_num == m)
//                 break;
//         } // End for - Scan list

//         // Found m neighbors 
//         if(neighbors_num == m)
//             break;
//     } // End for - Probes

//     // Nearest neighbor found 
//     if(found == 1){
//         nNeighbor = *iterNearestNeighbor;
//         if(neighborDistance != NULL)
//             *neighborDistance = minDist;
//     }
//     else{
//         nNeighbor.setId("Nearest neighbor not found");
//         if(neighborDistance != NULL)
//             *neighborDistance = minDist;
//     }
// }

};

		
		
		