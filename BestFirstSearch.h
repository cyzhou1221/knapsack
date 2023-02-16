#include <vector>
using namespace std;

// "item" struct for storing all respective values for a given item
typedef struct item
{
    int idBeforeSorting;  // item index before sorting, begin from 0
    int profit;
    int weight;
    float ppw;            // price per unit weight

    // constructors
    item(){}
    item(int id, int p, int w){
        idBeforeSorting = id;
        profit = p;
        weight = w;
        ppw = (float)p/(float)w;
    }
} item;

// "Node" struct for building the tree
typedef struct Node
{
    vector<int> idFixedItems_to_0;
    vector<int> idFixedItems_to_1;
    int weightFixedItems;    // weight of items fixed to 1
    int profitFixedItems;    // profit of items fixed to 1
    float bound;             // bound obtained by solving LP relaxation
    vector<float> lp_solution;
    bool isInt;    // indicate whether the lp_solution is integer
    int id;        // index of the critical item, [0, items_num-1]; if there isn't, -1
    
    // constructors
    Node(){}
    Node(vector<int> id_fixed_to_0, vector<int> id_fixed_to_1, int fixed_wgt, int fixed_pft){
        idFixedItems_to_0 = id_fixed_to_0;
        idFixedItems_to_1 = id_fixed_to_1;
        weightFixedItems = fixed_wgt;
        profitFixedItems = fixed_pft;
        bound = -1;
        lp_solution = {};
        isInt = false;
        id = -2;
    }
} Node;