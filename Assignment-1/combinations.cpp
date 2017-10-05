/*
	@ Author: Hao Huang
	@ Usage: In a command-line terminal, type: g++ -o combinations combinations.cpp, and then type: ./combinations 2 5
*/

#include <cstdlib>
#include <vector>
#include <iostream>
using namespace std;

/*
 * @ Print a vector to stdout.
 * @ Param: res - the vector to be printed
 * @ Return: void
 */
void print(const vector<int>& res){
    int len = res.size();
    for(int i = 0; i < len; i++){
        cout << res[i] << " ";
    }
    cout << endl;
}

/*
 * @ Calculate C(k, n) combinations on an integer from 1 to n.
 * @ Param: vec - an integer vector [1, 2, ..., n]
 * @ Param: res - a vector to store the result and it will be re-written every time when a new result is generated.
 * @ Param: k - input variable
 * @ Param: n - input variable
 * @ Param: iter - an iterator object to drive a loop on the vec
 * @ Param: res_idx - an index on the res to keep the position of the result
 */
void combination(const vector<int>& vec, vector<int>& res, int k, int n, vector<int>::iterator iter, int res_idx){
    if(k == 0){
        print(res);
        return;
    }

    int spos = iter - vec.begin();
	int epos = n - k;
    for(int i = spos; i <= epos; i++){
        res[res_idx] = vec[i];
        combination(vec, res, k - 1, n, ++iter, res_idx + 1);
    }
}


int main(int argc, char* argv[]){
    int k = atoi(argv[1]);
    int n = atoi(argv[2]);
  
	// Generate an integer vector from 1 to n
    vector<int> vec;
    for(int i = 0; i < n; i++){
        vec.push_back(i + 1);
    }

    vector<int> res(k);
    vector<int>::iterator iter = vec.begin();	// An iterator to drive a loop on vec
    combination(vec, res, k, n, iter, 0);

  return 0;
}
