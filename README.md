## :door: Problem Definition 
<strong>Picture(N) and Object(N)</strong> – are square matrices of integers with N rows and N columns. Each member of the matrix represents a “color”. The range of possible colors is [1, 100].<br/>
<strong>Position(I, J)</strong> defines a coordinates of the upper left corner of the Object into Picture.
<br/>
For each pair of overlapping members p and o of the Picture and Object we will calculate a relative difference<br/><br/>
                                           diff =  abs((p – o)/p)<br/><br/>
The total difference is defined as a sum of all relative differences for all overlapping members for given Position(I, J) of the Object into Picture. We will call it <strong>Matching</strong>(I, J).<br/><br/>
For example, for the Picture and Object the matching at Position(0,0) is equal<br/><br/>
Matching(0,0) = abs((10-5)/10) + abs((5-14)/5) + abs((67-9)/67) + abs((23-20)/23) + abs((6-56)/6) +
abs((5-2)/5) + abs((12-6)/12) + abs((10-10)/10) + abs((20-3)/20)

|10|5|67|12|8|4|
|---|---|---|---|---|
|23|6|5|14|9|5|
|12|10|20|56|2|3|
|1|2|6|10|3|2|
|45|3|7|5|5|2|
|11|43|2|54|1|12|





