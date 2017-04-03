imageRetrieval.py

team member: Liangji Wang, Shengjie Xue, Xinyi chen, Xuan Ding

Usage:
this is a python script, the way to use it is simply type 'python imageRetrieval.py'

input: point1(latitude1, longitude1)
       point2(latitude2, longitude2)
output: aerial imagery of the given bounding box

Script step:
1. First we judge whether the input is within the limit. If not, we take MinLat/MaxLat or MinLon/MaxLon as input.
2. Find the pixel coordinate for point1 and point2 according to their latitude and longitude.
3. Find the coordinate of tiles that points are located in.
4. In the bounding box, calculate all the quadKeys of tiles. Then download and stitch the sub-images for these tiles. 
5. Derive a more accurate image by cutting the stitched image according to the latitudes and longitudes input .

source: "https://msdn.microsoft.com/en-us/library/bb259689.aspx"




 

 
