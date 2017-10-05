(*
	@ Author: Hao Huang
	@ Usage: In a command-line terminal, type: ocamlc -o combinations combinations.ml, and then type: ./combinations 2 5
*)

open Printf

(*
	@ Print an array to stdout
	@ Param: arr - the array to be printed
*)
let rec print_array arr = 
	let pos = ref 0 in
	while !pos < Array.length arr do
		printf "%d" arr.(!pos);
		print_string " ";
		pos := !pos + 1
	done;
	print_string "\n";;

(*
	@ Calculate C(k, n) combinations on an integer from 1 to n.
	@ Param: arr - an integer array [1, 2, ..., n]
	@ Param: res - an array to store the result and it will be re-written every time when a new result is generated.
	@ Param: n - input variable
	@ Param: k - input variable
	@ Param: i - a for loop counter to drive a loop on the arr
	@ Param: r - an index on the res to keep the position of the result
*)
let rec combination arr res n k i r = 
	if k = 0 then print_array res
	else
		let spos = i in
		let epos = (n - k) in
		for j = spos to epos do
			res.(r) <- arr.(j);
			combination arr res n (k - 1) (j + 1) (r + 1);
		done;;

let main () =
	let argc = Array.length Sys.argv in
	if argc <> 3 then print_string "Please enter k and n.\n"
	else 
		let k = int_of_string Sys.argv.(1) in
	    let n = int_of_string Sys.argv.(2) in
		(* Initialize arr and allocate space for res *)
	    let arr = Array.init n (fun i -> i + 1) in
	    let res = Array.init k (fun i -> i) in
	    combination arr res n k 0 0;	(* Start recursion from the first element in arr *)
	
	exit 0;;

main ();;
	
