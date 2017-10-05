/*
    @ Author: Hao Huang
    @ Usage: In a command-line terminal, type: /u/cs254/bin/mcs combinations.cs, and then type: /u/cs254/bin/mono combinations.exe 2 5
*/

using System;
using System.Collections.Generic;

class Test{
    /*
        @ Print a vector to stdout.
        @ Param: list - the list to be printed
        @ Return: void
    */
	static void Print(List<int> list){
		foreach(int k in list){
			Console.Write(k + " ");
		}
		Console.Write(Environment.NewLine);
	}
	
    /*
        @ Calculate C(k, n) combinations on an integer from 1 to n.
        @ Param: list - an integer list [1, 2, ..., n]
        @ Param: res - a list to store the result and it will be re-written every time when a new result is generated.
        @ Param: k - input variable
        @ Param: n - input variable
        @ Param: i - a counter during iteration
        @ Param: res_idx - an index on the res to keep the position of the result
        @ Return: IEnumerable<List<int>>
    */
	static IEnumerable<List<int>> Combination(List<int> list, List<int> res, int k, int n, int spos, int res_idx){
		if(k == 0){
			yield return res;
			yield break;
		}
		int epos = n - k;
		for(int i = spos; i <= epos; i++){
			res[res_idx] = list[i];
			foreach(List<int> p in Combination(list, res, k - 1, n, i + 1, res_idx + 1)){
				yield return p;
			}
		}
	} 
	
	static void Main(string[] args){
		if(args.Length != 2){
			Console.WriteLine("Error! Please enter two arguments for k and n.\n");
		}
		
		int k = Int32.Parse(args[0]);
		int n = Int32.Parse(args[1]);
		
        // Generate an integer list containing from 1 to n
		List<int> list = new List<int>();
		for(int i = 0; i < n; i++){
			list.Add(i + 1);
		}

        // Allocate space for result list
		List<int> res = new List<int>();
		for(int i = 0; i < k; i++){
			res.Add(0);
		}
        
        // Iteration
		foreach(List<int> s in Combination(list, res, k, n, 0, 0)){
			Print(s);
		}
	}
}
