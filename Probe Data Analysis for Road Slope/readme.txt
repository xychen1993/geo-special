Input are two files:
  Partition6467ProbePoints.csv
  Partition6467LinkData.csv

We use “mapping.cpp” to load data and match probe points with links data and store the output in a new file “mapped.txt”.

mapping.cpp
1. Load link data
For each line in LinkData file, we store the original information in a newly defined array “Link”. 

2. Match links with probe points    
Find the link for a probe point by calculating the smallest distance between the point and the all the partial links which have been created before.

3. Use “Sample ID” to simplify the process.

Output format :
	sampleID, dateTime, sourceCode, latitude, longitude, altitude, speed, heading, linkPVID, direction, distFromRef, distFromLink

		sampleID	is a unique identifier for the set of probe points that were collected from a particular phone.
		dateTime	is the date and time that the probe point was collected.
		sourceCode	is a unique identifier for the data supplier (13 = Nokia).
		latitude	is the latitude in decimal degrees.
		longitude	is the longitude in decimal degrees.
		altitude	is the altitude in meters.
		speed		is the speed in KPH.
		heading		is the heading in degrees.
		linkPVID	is the published versioned identifier for the link.
		direction	is the direction the vehicle was travelling on thelink (F = from ref node, T = towards ref node).
		distFromRef	is the distance from the reference node to the map-matched probe point location on the link in decimal meters.
		distFromLink	is the perpendicular distance from the map-matched probe point location on the link to the probe point in decimal meters.





calculate_slope.py
1. Load matched data
Read all the mapping data derived from first step, since one link can be matched by many porbe points, store probe points by link id.

2. Derive road slope for each road link
We can derive a slope using two probe points, for each link, there are many matched probe points, so we can derive many slopes(using each two probe points).

3. Evaluate the derived road slope with the surveyed road slope in the link data file
Calculate the average of derived slopes for each link, evaluate average slope with the surveyed road slope. If the difference between them is greater than 2 degrees, consider the derived road slope is wrong.

Output format :
'average error'
'bad error count'
'total error count'
'max slope error'
'maxError'