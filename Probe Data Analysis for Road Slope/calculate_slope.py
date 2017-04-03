import math
probSets = {}

#calculate the slope using two points
def calculateSlope(point1, point2):
    dlon = point1[0] - point2[0]
    dlat = point1[1] - point2[1]
    a = math.sin(dlat/2)**2 + math.cos(point2[1]) * math.cos(point1[1]) * math.sin(dlon/2)**2
    dist = 2 * math.atan2(a**0.5, (1-a)**0.5) * 6371000
    if dist == 0:
        return None
    return math.atan((point2[2] - point1[2])/dist) * 180 / math.pi

# Load matched data, store probe points by link id.
readLines = open("allData.txt").readlines()
for line in readLines:
    cols = line[:-1].split(",")
    latitude = float(cols[3])
    longtitude = float(cols[4])
    altitude = float(cols[5].strip())
    linkid = cols[8].strip()
    if altitude != None and altitude != "":
        dist = float(cols[-1].strip())
        if linkid not in probSets:
            probSets[linkid] = []
        probSets[linkid].append([longtitude, latitude, altitude, dist])#Put them in to probeSets

#Derive road slope for each road link
slopes = {}
for linkid in probSets:
    probes = probSets[linkid]
    probes.sort(key=lambda x: x[3]) #sort probes by distance
    for i in range(1, len(probes)):
        slope = calculateSlope(probes[i-1], probes[i])
        if slope != None:
            if linkid not in slopes:
                slopes[linkid] = []
            slopes[linkid].append([slope, probes[i][3]])
            #Slope(slope, probes[i].dist)

# Evaluate the derived road slope with the surveyed road slope in the link data file
readLines = open("Partition6467LinkData.csv").readlines()
sumError = 0.0
deviationSum = 0.0
errorCount = 0
testedNumber = 0
maxSlope = 0.0
minSlope = 100.00
for line in readLines:
    cols = line[:-1].split(',')
    slopeinfoCol = cols[-1].strip()
    if slopeinfoCol == "":
        continue
    linkid = cols[0]
    if linkid not in slopes:
        continue
    slope = slopeinfoCol.split('|')
    for slopeinfo in slope:
        slopeinfo = slopeinfo.split('/')
        realDist = float(slopeinfo[0])
        realSlope = float(slopeinfo[1])
        slopesSum = 0
        slopeCount = 0
        #Calculate the average estimation of derived slopes
        for slope in slopes[linkid]:
            if abs(slope[1] - realDist) < 8:
                # remove the invalid slope
                if abs(slope[0]) > 30: 
                    continue
                slopesSum += slope[0]
                slopeCount += 1
        if slopeCount != 0:
            calSlope = slopesSum/slopeCount
            sumError += abs(calSlope - realSlope)
            if realSlope != 0:
                deviationSum += abs(calSlope - realSlope)/realSlope
            testedNumber += 1
            maxSlope = max(maxSlope, calSlope)
            minSlope = min(minSlope, calSlope)

            # if calculated slope - real slope > 2, consider it as error estimate
            if abs(calSlope - realSlope) > 2:
                errorCount += 1

print 'number of tested probe points: ',testedNumber
print 'number of error derived road slopes: ', errorCount
print 'error rate: ', errorCount * 1.0/testedNumber * 100,'%'
print 'average error: ', sumError/testedNumber
#print 'average deviation: ', deviationSum/testedNumber
print 'Range of derived slope: ', minSlope,'to',maxSlope

