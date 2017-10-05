/*
	@ Author: Hao Huang
    @ Usage: In command-line terminal, type: swipl, and then type: '[combinations].' to load this file, and then type: 'combinations(2, 5, L).' to view one result. To get all results, please continue to type ';' until the end.
*/

/* Append an element to array*/
append([], X, X).
append([X|Y], Z, [X|W]) :- append(Y, Z, W).

/* From any array, the result of select 0 element is an empty array */
startFrom(0, [], _).
/*
    Selecting K elements from an array can be divide into two sub-tasks:
    1. Select the first element and then select K - 1 elements from the array excluded the first element (T);
    2. Select K elements from the array excluded the first element (T).
*/
startFrom(K, L, [H|T]) :- K > 0, K1 is K - 1, startFrom(K1, P, T), append([H], P, L); K > 0, startFrom(K, L, T).

/* Generate an array [1, ..., n] and begin selection */
combinations(K, N, L) :- findall(Num, between(1, N, Num), V), startFrom(K, L, V).

