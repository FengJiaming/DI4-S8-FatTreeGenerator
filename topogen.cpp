/*************************************************
Copyright:Polytech Tours
Author: XU Tianjiao && FENG Jiaming && GUO Xiaoqing
Date:2019-05-03
Title: Building a Fat-Tree Topology
Description: 
	The Fat-Tree topology consists of 3 levels: Edge, Aggregation, and Core.
	To build such a topology within a data center, rules have already been defined that will be listed below:
	<1> Define the parameter k which represents the number of pods (Point of delivery is a module of the network ex: calculation machines, storage ...), of which each pod contains (k / 2) ² servers and 2 levels of k / 2 switches with k ports.
	<2> Each Edge-level switch can connect up to k / 2 servers and k / 2 aggregation switches.
	<3> Each aggregation switch can connect up to k / 2 level and core level switch.
	<4> (k / 2) ² Core level switches each of which connects k pods.

	The program will take a parameter k which will vary between 4,6,8,10, ... and it must return a file (For example: result.topo) describing the fat-tree topology according to k.
	The program must be compilable by Unix command like this example:
	\# g++ topogen.cpp -o generator
	\# ./generator 4
**************************************************/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>

using namespace std;

class topologie {
private:
	ofstream file;
	int k; //the parameter k: number of the pod
	int num_pod; //number of pod
	int num_edge; //number of edge switches per pod
	int num_agre; //number of agre switches per pod
	int num_host; //number of hosts per edge switch
	int num_port; //number of ports

public:
	topologie(int k) {

		this->k = k;
		this->num_pod = k;
		this->num_edge = (k / 2);
		this->num_agre = this->num_edge;
		this->num_host = (k / 2);
		this->num_port = k;

		// Generate the file header
		file.open("resultat.topo");
		file << "#fat tree topology file." << endl;
		file << "#Value of k = " << k << endl;
		file << "#Total number of hosts = " << pow(k / 2, 2) * k << endl;
		file << "#Number of hosts under each switch = " << k / 2 << endl;
		file << "####################################################" << endl << endl;
	}


	/*************************************************
	Function: printNode()
	Description: print all nodes and its informations.
	Output: (For example: k=4)
			Hca 1 "Node(0)" ==> 'Type' 'total number of ports' 'name of host[node]'
			[1] "Edge(0 0 1)"[2] ==> 'N°port of the host[node] connected' 'name of the device[edge] to connect' 'N°port of device[edge] to connect'
	*************************************************/
	void printNode() {
		
		int idx = 0;	//To store the number of nodes

		//Traversing each pod
		for (int i = 0; i < num_pod; i++) 
		{
			//Traversing each edge switch of each pod
			for (int j = 0; j < num_edge; j++) 
			{
				//Traversing each host of each edge switch
				for (int t = 0; t < num_host; t++) 
				{
					file << "Hca\t1\t\"Node(" << idx << ")\"" << endl;
					file << "[1]  \"Edge(" << i << " " << j << " 1)\"[" << (t + 1) * 2 << "]" << endl;
					idx++;
				}
			}
		}
	}


	/*************************************************
	Function: printEdge()
	Description: print all edges and its informations.
	Output: (For example: k=4)
			Switch 4 "Edge(0 0 1)" ==>  'Type' 'total number of ports' 'Edge(n°pod, n°switch in this pod,1)'
			[1] "Aggr(0 2 1)"[2] ==> 'n°port of the [Edge] connected' 'Aggr(n°pod, n°switch in this pod,1)' 'n°port of the [Aggregation] to connect'
			[2] "Node(0)"[1] ==> 'n°port of the [Edge] connected' 'name of [node]' 'n°port of the [Node] to connect'
			[3] "Aggr(0 3 1)"[2] ==> 'n°port of the [Edge] connected' 'Aggr(n°pod, n°switch in this pod,1)' 'n°port of the [Aggregation] to connect'
			[4] "Node(1)"[1] ==> 'n°port of the [Edge] connected' 'name of [node]' 'n°port of the [Node] to connect'
	*************************************************/
	void printEdge() {
		int idx = 0;	//To store the number of nodes

		//Traversing each pod
		for (int i = 0; i < num_pod; i++) 
		{
			//Traversing each edge switch of each pod
			for (int j = 0; j < num_edge; j++) 
			{
				//print each edge switch
				file << "Switch\t\t" << num_port << "\t\"Edge(" << i << " " << j << " 1)\"" << endl;


				int idy = num_pod / 2;	//To store the number of the agre switch connected to the edge switch
				
				//Traversing each port of each edge switch
				for (int t = 0; t < num_port; t++) 
				{
					//Odd number port is connected to agre switch
					if (t % 2 == 0) {
						file << "[" << t + 1 << "]" << " \"Aggr(" << i << " " << idy << " 1)\"[" << (j + 1) * 2 << "]" << endl;
						idy++;
					}
					//Even serial port is connected to the host
					else {
						file << "[" << t + 1 << "]" << " \"Node(" << idx << ")\"[1]" << endl;
						idx++;
					}
				}

			}
		}
	}


	/*************************************************
	Function: printAgre()
	Description: print all aggregations and its informations.
	Output: (For example: k=4)
			Switch 4 "Aggr(3 3 1)" ==>  'Type' 'total number of ports' 'Aggr(n°pod, n°switch in this pod,1)'
			[1] "Core(4 2 1)"[4] ==> 'n°port of the [Aggregation] connected' 'Core(k, n°group, n°Core in the group)' 'n°port of the [Core] to connect'
			[2] "Edge(3 0 1)"[3] ==> 'n°port of the [Aggregation] connected' 'Edge(n°pod, n°switch in this pod,1)' 'n°port of the [Edge] to connect'
			[3] "Core(4 2 2)"[4] ==> 'n°port of the [Aggregation] connected' 'Core(k, n°group, n°Core in the group)' 'n°port of the [Core] to connect'
			[4] "Edge(3 1 1)"[3] ==> 'n°port of the [Aggregation] connected' 'Edge(n°pod, n°switch in this pod,1)' 'n°port of the [Edge] to connect'
	*************************************************/
	void printAgre() {
		
		//Traversing each pod
		for (int i = 1; i <= k; i++) 
		{
			//Traversing each agre switch
			for (int j = 1; j <= num_agre; j++) 
			{
				//print each agre switch
				file << "Switch\t\t" << num_port << "\t\"Aggr(" << i - 1 << " " << j + k / 2 - 1 << " 1)\"" << endl;
				for (int m = 1; m <= k; m++) 
				{
					if (m % 2 == 1)
						file << "[" << m << "]  \"Core(" << k << " " << j << " " << m / 2 + 1 << ")\"[" << i << "]" << endl;
					else
						file << "[" << m << "]  \"Edge(" << i - 1 << " " << m / 2 - 1 << " 1)\"[" << j * 2 - 1 << "]" << endl;
				}
			}
		}

	}


	/*************************************************
	Function: printAgre()
	Description: print all aggregations and its informations.
	Output: (For example: k=4)
			Switch 4 "Core(4 1 1)" ==>  'Type' 'total number of ports' 'Core(k, n°group, n°Core in the group)'
			[1] "Aggr(0 2 1)"[1] ==> 'n°port of the [Core] connected' 'Aggr(n°pod, n°switch in this pod,1)' 'n°port of the [Aggregation] to connect'
			[2] "Aggr(1 2 1)"[1] ==> 'n°port of the [Core] connected' 'Aggr(n°pod, n°switch in this pod,1)' 'n°port of the [Aggregation] to connect'
			[3] "Aggr(2 2 1)"[1] ==> 'n°port of the [Core] connected' 'Aggr(n°pod, n°switch in this pod,1)' 'n°port of the [Aggregation] to connect'
			[4] "Aggr(3 2 1)"[1] ==> 'n°port of the [Core] connected' 'Aggr(n°pod, n°switch in this pod,1)' 'n°port of the [Aggregation] to connect'
	*************************************************/
	void printCore() {
		//generate core switch, divise in k/2 groups, each group has k/2 coreSwitch, one iteration generate two cores
		for (int i = 1; i <= k / 2; i++) 
		{
			for (int j = 1; j <= k / 2; j++) 
			{
				file << "Switch\t\t" << num_port << "\t\"Core(" << k << " " << i << " " << j << ")\"" << endl;
				for (int m = 1; m <= k; m++) 
				{
					file << "[" << m << "]  \"Aggr(" << m - 1 << " " << k / 2 + i - 1 << " 1)\"[" << 2 * j - 1 << "]" << endl;
				}
			}
		}
	}


	/*************************************************
	Function: closeFile()
	Description: close the file
	*************************************************/
	void closeFile() {
		file.close();
	}
};


int main(int argc, char* argv[])
{
	int k;
	if (argc > 1) {
		k = atoi(argv[1]);

		//Check if k is legal
		if (k < 4 || k % 2 == 1) {
			cout << "Wrong k" << endl;
			return 1;
		}
		else {

			topologie *topo = new topologie(k);
			
			topo->printNode();
			topo->printEdge();
			topo->printAgre();
			topo->printCore();

			topo->closeFile();
		}
	}
	else {
		cout << "Not enough params" << endl;
		return 1;
	}

	return 0;
}

