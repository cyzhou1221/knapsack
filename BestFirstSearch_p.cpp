#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <ctime>
#include <pthread.h>
#include "BestFirstSearch.h"
#define NUM_THREADS 36


//---------------------Parallel Version------------------//

//----------- Global Variables -------------//
int itemsCount;
int weightCapacity;
vector<item> items;
// Branch-and-Bound related
int lowerBound;
vector<int> solution;
int nodesCount;
vector<Node> queue;
// Parallel related
pthread_attr_t attr;
pthread_t threads[NUM_THREADS];
pthread_mutex_t mutex_1, mutex_2, mutex_3; // for lowerBound and solution & queue & nodesCount 


//----------- Utility Functions ------------//

// Comparator for sorting items by profit per unit weight
bool cmpr_items(const item& l, const item& r)
{
    return l.ppw > r.ppw;
}

// Comparator for ordering nodes in priority queue by bounds
bool cmpr_nodes(const Node& l, const Node& r)
{
    return l.bound < r.bound;
}

// Judge if index 'id' is in idItems
bool inFixedItems(int id, const vector<int>& idItems)
{
    bool flag = false;
    for(vector<int>::const_iterator iter = idItems.begin(); iter != idItems.end(); ++iter){
        if(*iter == id){
            flag = true;
            break;
        }
    }
    return flag;
}

// Read the problem from 'filename'
bool getInput(string filename)
{
    ifstream infile(filename);
    istringstream ss, num_as_str;
    string line, str;   
    int p, w;      // store the profit and weight temporarily

    if(!infile){
        return false;
    }

    // Parse the first line: itemsCount, weightCapacity
    getline(infile, line);  // store a line of 'filename' in `line`
    ss.clear();             // reset the state of ss
    ss.str(line);           // reset the content of ss
    getline(ss, str, ',');  // store data of 'ss' in 'str' with ',' as delimiter
    num_as_str.clear();
    num_as_str.str(str); 
    num_as_str >> itemsCount;
    getline(ss, str, ','); 
    num_as_str.clear();  
    num_as_str.str(str); 
    num_as_str >> weightCapacity;

    // Parse other lines: profit, weight
    items = {};
    for(int i = 0; i < itemsCount; i++){
        getline(infile, line);
        ss.clear();          
        ss.str(line);       
        getline(ss, str, ',');
        num_as_str.clear();
        num_as_str.str(str);
        num_as_str >> p;
        getline(ss, str, ',');
        num_as_str.clear();
        num_as_str.str(str);
        num_as_str >> w;
        // create new item
        items.push_back(item(i, p, w));
    }
    return true;
}

/* solve the LP Relaxation of Node 'n', retore the upper bound in n.bound
 * return value:
 *        true: LP relaxation feasible
 *       false: LP relaxation infeasible
*/
bool solveLP(Node& n)
{
    // assume items[] has been sorted by profit per unit weight decreasingly
    int weight = n.weightFixedItems;
    float bound = float(n.profitFixedItems);

    // LP relaxation infeasible
    if(weight > weightCapacity){ 
        return false;
    }

    // solve the LP relaxation, calculate the bound and fill up n.lp_solution[0--id]
    bool flag = false;  // indicate whether the LP solution is integer
    int id = -1;        // index of the critical item, [0, items_num-1]; if there isn't, -1
    for(int i = 0; i < itemsCount; i++){
        if(inFixedItems(i, n.idFixedItems_to_0)){
            n.lp_solution.push_back(0);
        }
        else if(inFixedItems(i, n.idFixedItems_to_1)){
            n.lp_solution.push_back(1);
        }
        else{         // weight <= weightCapacity is always true
            if(weight + items[i].weight <= weightCapacity){ // if the items[i] can be added into the knapsack, add it
                n.lp_solution.push_back(1);
                weight += items[i].weight;
                bound += float(items[i].profit);
            }
            else{     // weight <= weightCapacity < weight + items[i].weight
                id = i; // record the index of the critical item
                if(weight == weightCapacity){
                    flag = true;
                    n.lp_solution.push_back(0);
                }
                else{ // weight < weightCapacity < weight + items[i].weight
                    float frac = float(weightCapacity - weight)/float(items[i].weight);
                    n.lp_solution.push_back(frac);
                    bound += frac*items[i].profit;
                }
                break;
            }
        }
    }

    if(id == -1){ // all of unfixed items have been set to 1
        flag = true;
    }
    else{         // n.lp_solution[0--id] has been filled up, fill up the rest
        for(int j = id + 1; j < itemsCount; j++){
            if(inFixedItems(j, n.idFixedItems_to_1)){
                n.lp_solution.push_back(1);
            }
            else{
                n.lp_solution.push_back(0);
            }
        }
    }

    n.bound = bound;
    n.isInt = flag;
    n.id = id;
    return true;
}


void *Node_Processing(void *n)
{
    int lBound;
    Node *v = (Node *)n;
    bool isFeasible; // indicate whether the LP is feasible

    pthread_mutex_lock(&mutex_1);
    lBound = lowerBound;
    pthread_mutex_unlock(&mutex_1);
    if(v->bound > lBound){ // else, prune by bound, do nothing  
        // Branch: v->lp_solution must not be integer
        pthread_mutex_lock(&mutex_3);
        nodesCount += 2;
        pthread_mutex_unlock(&mutex_3);
        // Choose 1: Fixed x[v->id] to 1
        Node u(v->idFixedItems_to_0, v->idFixedItems_to_1, v->weightFixedItems, v->profitFixedItems);  // generate a child
        u.idFixedItems_to_1.push_back(v->id);
        u.weightFixedItems += items[v->id].weight;
        u.profitFixedItems += items[v->id].profit;
        isFeasible = solveLP(u);
        pthread_mutex_lock(&mutex_1);
        lBound = lowerBound;
        pthread_mutex_unlock(&mutex_1);
        if(isFeasible && (u.bound > lBound)){    // if LP feasible and bound satisfies, else, prune 
            if(u.isInt){                         // LP optimal solution is integer, prune by optimality
                pthread_mutex_lock(&mutex_1);
                lowerBound = int(u.bound);       // update global lower bound and feasible solution
                for(int i = 0; i < itemsCount; i++){
                    solution[i] = int(u.lp_solution[i]);
                }
                pthread_mutex_unlock(&mutex_1);
            }
            else{
                pthread_mutex_lock(&mutex_2);
                queue.push_back(u);
                push_heap(queue.begin(), queue.end(), cmpr_nodes); // make a heap, by default, it's a max heap
                pthread_mutex_unlock(&mutex_2);
            }
        }
        // Choose 2: Fixed x[v->id] to 0
        Node w(v->idFixedItems_to_0, v->idFixedItems_to_1, v->weightFixedItems, v->profitFixedItems);  // generate a child
        w.idFixedItems_to_0.push_back(v->id);
        isFeasible = solveLP(w);
        pthread_mutex_lock(&mutex_1);
        lBound = lowerBound;
        pthread_mutex_unlock(&mutex_1);
        if(isFeasible && (w.bound > lBound)){   // if LP feasible and bound satisfies, else, prune
            if(w.isInt){                        // LP optimal solution is integer, prune by optimality 
                pthread_mutex_lock(&mutex_1);
                lowerBound = int(w.bound);      // update global lower bound and feasible solution
                for(int i = 0; i < itemsCount; i++){
                    solution[i] = int(w.lp_solution[i]);
                }
                pthread_mutex_unlock(&mutex_1);
            }
            else{
                pthread_mutex_lock(&mutex_2);
                queue.push_back(w);
                push_heap(queue.begin(), queue.end(), cmpr_nodes); // make a heap, by default, it's a max heap
                pthread_mutex_unlock(&mutex_2);
            }
        }
    }
    pthread_exit(NULL);
}

/* return value: 
 *      0: Success
 *     -1: Root node LP relaxation infeasible
 */
int BFS_Branch_and_Bound()
{
    clock_t startTime, endTime;
    time_t start, end;
    int i; 
    int threadSum, loopCount;

    // Initialize
    lowerBound = 0;     // global lower bound
    solution = {};      // initial feasible solution 
    for(i = 0; i < itemsCount; i++){
        solution.push_back(0);
    }

    queue = {};               // Priority queue
    Node root({}, {}, 0, 0);  // Root node
    nodesCount = 1;

    // Solve Root node LP
    if(!solveLP(root)){ // Root node LP relaxation infeasible, prune   
        return -1;
    }
    if(root.bound <= lowerBound){ // prune by bound, current solution is optimal, prune
        return 0;
    }
    if(root.isInt){                   // LP optimal solution is integer, prune by optimality
        lowerBound = int(root.bound); // update global lower bound and feasible solution
        for(i = 0; i < itemsCount; i++){
            solution[i] = int(root.lp_solution[i]);
        }
        return 0;
    }
    queue.push_back(root); // if not be pruned, put into queue

    // Queue
    // Set the attribute of thread and initialize three mutexes
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutex_1, NULL);
    pthread_mutex_init(&mutex_2, NULL);
    pthread_mutex_init(&mutex_3, NULL);
    Node nodesArray[NUM_THREADS];
    threadSum = 0;
    loopCount = 0;
    startTime = clock();
    time(&start);
    while(!queue.empty()){ // when the priority queue is empty, we have the solution
        loopCount++;
        int num = min(int(queue.size()), NUM_THREADS);
        threadSum += num; 
        // Fill up the array and create threads to begin Node_Processing
        for(i = 0; i < num; i++){
            // Get the first node, with largest upper bound
            pthread_mutex_lock(&mutex_2);
            pop_heap(queue.begin(), queue.end(), cmpr_nodes);
            nodesArray[i] = queue.back();
            queue.pop_back();
            pthread_mutex_unlock(&mutex_2);
            pthread_create(&threads[i], &attr, Node_Processing, (void *)&(nodesArray[i]));
        }
        // Wait until all threads exit
        for(i = 0; i < num; i++){
            pthread_join(threads[i], NULL);
        }
    }
    time(&end);
    endTime = clock();
    cout << "PAR: The node processing's real time is: " << setprecision(6) << difftime(end, start) << " secs";
    cout << ", CPU time is: " << setprecision(6) << (double)(endTime - startTime)/CLOCKS_PER_SEC << " secs";
    cout << ", nodes count is: " << nodesCount << endl;
    cout << "     Average number of threads is " << setprecision(6) << (double)(threadSum)/loopCount << endl;

    // Destroy mutexes and attribute of thread 
    pthread_mutex_destroy(&mutex_3);
    pthread_mutex_destroy(&mutex_2);
    pthread_mutex_destroy(&mutex_1);
    pthread_attr_destroy(&attr);
    
    return 0;
}

int main(int argc, char **argv)
{
    // Get input arguments
    if(!(argc == 3)){
        cout << "Usage: program inputfile outputfile" << endl;
        return 0;
    }
    string inputfile = argv[1];
    string outputfile = argv[2];

    // Read the problem
    if(!getInput(inputfile)){
        cout << "Can't find the file '" << inputfile << "'!\n";
        return 0;
    }
    cout << "Max threads count is: " << NUM_THREADS << endl;
    cout << "Read problem from: " << inputfile << endl;

/*
    // Display input information
    cout << "Input information: " << endl;
    cout << "size of problem: " << itemsCount << endl;
    cout << "capacity of weight: " << weightCapacity << endl;
    for(int i = 0 ; i < items.size(); i++){
        cout << "item " << items[i].idBeforeSorting << ": weight " << items[i].weight << ", profit " << items[i].profit << endl;
    }
*/
    
    // Sort by decreasing profit per unit weight
    sort(items.begin(), items.end(), cmpr_items);

    // Solve the problem by Branch and Bound
    int flag = BFS_Branch_and_Bound();
    if(flag == -1){
        cout << "LP relaxation infeasible!" << endl;
    }
    else{
        // Print result to outputfile
        ofstream fp(outputfile);  
        fp << "Read problem from: " << inputfile << endl;
        fp << "problem size: " << itemsCount << endl;
        fp << "nodes count: " << nodesCount << endl;
        fp << "optimal value: " << lowerBound << endl;
        // Change to original solution
        int *original_solution = new int[itemsCount];
        for(int i = 0; i < itemsCount; i++){
            original_solution[items[i].idBeforeSorting] = solution[i];
        }
        fp << "optimal solution: " << endl;
        for(int i = 0; i < itemsCount; i++){
            fp << "x_" << i+1 << "," << original_solution[i] << endl;
        }

        fp.close();
        delete[] original_solution;
        original_solution = NULL;
    }
    
    return 0;
}
