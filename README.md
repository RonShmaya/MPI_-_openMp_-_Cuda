## :door: Problem Definition 
Picture(N) and Object(N) – are square matrices of integers with N rows and N columns. Each member of the matrix represents a “color”. The range of possible colors is [1, 100].
Position(I, J) defines a coordinates of the upper left corner of the Object into Picture. 
For each pair of overlapping members p and o of the Picture and Object we will calculate a relative difference
				diff =  abs((p – o)/p)
The total difference is defined as a sum of all relative differences for all overlapping members for given Position(I, J) of the Object into Picture. We will call it Matching(I, J).
For example, for the Picture and Object from the Fig.1 the matching at Position(0,0) is equal
Matching(0,0) = abs((10-5)/10) + abs((5-14)/5) + abs((67-9)/67) + abs((23-20)/23) + abs((6-56)/6) +
abs((5-2)/5) + abs((12-6)/12) + abs((10-10)/10) + abs((20-3)/20)
