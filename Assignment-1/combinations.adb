-- @Author: Hao Huang
-- @Usage: In a command-line terminal, type: gnatmake combinations.ada,
--	   and then type: ./combinations 2 5

with Ada.Text_IO, Ada.Integer_Text_IO, Ada.Command_Line, Ada.Containers.Vectors;
use Ada.Text_IO, Ada.Integer_Text_IO, Ada.Command_Line, Ada.Containers;

-- @ This is the main procedure and acts as an entry.
procedure combinations is
	
	package Integer_Vectors is new Vectors(Natural, Integer);

	-- @ Print a vector to stdout
	-- @ Param: the vector to be printed
	procedure Print(Vec : in Integer_Vectors.Vector) is
		Cursor : Integer_Vectors.Cursor;
	begin
		Cursor := Vec.First;
        	while Integer_Vectors.Has_Element(Cursor) loop
                	Put(Integer_Vectors.Element(Cursor));
			Integer_Vectors.Next(Cursor);
        	end loop;
		New_Line;
	end Print;

	-- @ Calculate C(k, n) combinations on an integer from 1 to n.
	-- @ Param: Vec - an integer vector [1, 2, ..., n]
	-- @ Param: Res - a vector to store the result and it will be re-written every time when a new result is generated.
	-- @ Param: k - input variable
	-- @ Param: n - input variable
	-- @ Param: Curosr - something like an iterator to dirve a loop on the Vec
	-- @ Param: res_idx - an index on the Res to keep the position of the result
	procedure process(Vec : in Integer_Vectors.Vector;
				Res : in out Integer_Vectors.Vector;
				k : in Integer;
				n : in Integer;
				Cursor : in out Integer_Vectors.Cursor;
				res_idx : in Integer) is

	epos : Integer;
	i : Integer;
	begin
		if k = 0 then
			Print(Res);
			return;
		end if;

		epos := n - k;
		i := Integer_Vectors.To_Index(Cursor);
		while i <= epos loop
			Res(res_idx) := Vec(i);
			i := i + 1;
			Cursor := Integer_Vectors.To_Cursor(Vec, i);
			process(Vec, Res, k - 1, n, Cursor, res_idx + 1);
		end loop;

	end process;
	
	k : Integer; 
	n : Integer;
	Count : Integer;
	Vec : Integer_Vectors.Vector;
	Res : Integer_Vectors.Vector;
	Cursor : Integer_Vectors.Cursor;

begin
	k := Integer'Value(Argument(Number => 1));
	n := Integer'Value(Argument(Number => 2));

	-- Generate an integer vector from 1 to n
	Count := 0;
	while Count < n loop
		Vec.append(Count + 1);
		Count := Count + 1;
	end loop;

	-- Allocate space for Res
	Count := 0;
	while Count < k loop
		Res.append(0);
		Count := Count + 1;
	end loop;

	Cursor := Vec.First;
	process(Vec, Res, k, n, Cursor, 0); -- Drive a loop
	
end combinations;
