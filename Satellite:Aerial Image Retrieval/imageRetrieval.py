# private const double EarthRadius = 6378137;
# private const double MinLatitude = -85.05112878;
# private const double MaxLatitude = 85.05112878;
# private const double MinLongitude = -180;
# private const double MaxLongitude = 180;
# Level of detail, from 1 (lowest detail) to 23 (highest detail)
# work cited: "https://msdn.microsoft.com/en-us/library/bb259689.aspx"

import sys
import json
import urllib
import math
import io
from PIL import Image

# function for get the proper value for n 
def Clip(n, minV, maxV):
    return min(max(n, minV), maxV)

# function for get map size
def mapSize(curLevel):
    return 256*2**curLevel

# function for convert input string to float latitude value
def getLat(val1, val2):
    val1 = float(val1 )
    val2 = float(val2)
    if val1 > val2:
        temp = val1
        val1 = val2
        val2 = temp
    if val1 < -85.05112878:
        print "warning: %s lower than lowest latitude" %(val1)
        val1 = -85.05112878
    if val2 > 85.05112878:
        print "warning: %s larger than largest latitude" %(val2) 
        val2 = 85.05112878
    return val1, val2

# function for convert input string to float longitude value
def getLong(val1, val2):
    val1 = float(val1)
    val2 = float(val2)
    if val1 > val2:
        temp = val1
        val1 = val2
        val2 = temp
    if val1 < -180:
        print "warning: %s lower than lowest longitude" %(val1)
        val1 = -180.0
    if val2 > 180:
        print "warning: %s larger than largest latitude" %(val2) 
        val2 = 180
    return val1, val2

# function convert lat and long value to pixel value
def latLongToPixelXY(lat, long, curLevel):
    x = (long + 180.0)/360.0
    sinLat = math.sin(lat * math.pi/180.0)
    y = 0.5 - math.log((1 + sinLat)/(1 - sinLat)) / (4 * math.pi)
    mapsize = mapSize(curLevel)
    pixelX = int(Clip(x * mapsize + 0.5, 0, mapsize - 1))
    pixelY = int(Clip(y * mapsize + 0.5, 0, mapsize - 1))
    return pixelX, pixelY

# function convert pixel value to tile value
def pixelXYToTileXY(x, y):
    return x / 256, y / 256

# function convert tile value to pixel value
def tileXYToPixelXY(tx, ty):
    return tx * 256, ty * 256

# function convert tile value to quadkey
def tileXYToQuadKey(tx, ty, curLevel):
    quadKey = ''
    for i in range(curLevel, 0, -1):
        digit = 0
        mask = 1 << (i - 1)
        if(tx & mask) != 0:
            digit += 1
        if(ty & mask) != 0:
            digit += 2
        quadKey += str(digit)
    return quadKey

# check whether two image are the same
def equal(image1,image2):
    if image1.size !=image2.size:
       return False
    for i in range(0, image1.width):
        for j in range(0, image1.height):
            if image1.getpixel((i, j)) !=image2.getpixel((i, j)):
                return False
    return True

# get quadkey list from the two points
def allquads(tX1,tY1,tX2,tY2,curlevel):
    AllQuads=[]
    for i in xrange(tY1, tY2+1):
        r = []
        for j in xrange(tX1, tX2+1):
            j %= 2**curlevel
            r.append(tileXYToQuadKey(j,i,curlevel))
        AllQuads.append(r)
    return AllQuads

# input interaction
def getUserInput():
    lat1 = raw_input("plz input first latitude: ")
    long1 = raw_input("plz input first longitude: ")
    lat2 = raw_input("plz input second latitude: ")
    long2 = raw_input("plz input second longitude: ")
    return inputProcessing(lat1, long1, lat2, long2)

# input sanitation
def inputProcessing(lat1, long1, lat2, long2):
    lat1 = float(lat1)
    long1 = float(long1)
    lat2 = float(lat2)
    long2 = float(long2)
    lat1, lat2 = getLat(lat1, lat2)
    long1, long2 = getLong(long1, long2)
    return lat1, long1, lat2, long2

if __name__ == "__main__":
    maxLevel = 23
    minLatitude = -85.05112878
    maxLatitude = 85.05112878
    minLongitude = -180
    maxLongitude = 180
    key = urllib.urlencode({"key":"Av6_H8GIYQyP-DLQwLOKDknW64QfmVgJmVpfiSO861v0x_j1pLPCOW6s-70nCzEW"})
    print 'Argument List:', str(sys.argv)
    if (len(sys.argv) == 5):
        try:
            lat1, long1, lat2, long2 = inputProcessing(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
        except:
            lat1, long1, lat2, long2 = getUserInput()
        
    else:
        lat1, long1, lat2, long2 = getUserInput()
    print 'lat1 %s, long1 %s, lat2 %s, long2 %s' %(lat1, long1, lat2, long2)
    IVimage = Image.open('test.jpeg')
    level = maxLevel
    # find the available level
    while level >= 0:
        pX1, pY1 = latLongToPixelXY(lat1, long1, level)
        tX1, tY1 = pixelXYToTileXY(pX1, pY1)
        quadKey = tileXYToQuadKey(tX1, tY1, level)
        request = "http://h0.ortho.tiles.virtualearth.net/tiles/h%s.jpeg?g=131&%s" % (quadKey, key)
        url = urllib.urlopen(request)
        tempImage = Image.open(io.BytesIO(url.read()))
        if equal(tempImage, IVimage):
            print 'level %s is invalid' %(level)
            level -= 1
        else:
            break
    print 'The level is %d . ' % level
    if level <=0 :
        print 'error: level is smaller than zero. '
        exit()

    pX2, pY2 = latLongToPixelXY(lat2, long2, level)
    tX2, tY2 = pixelXYToTileXY(pX2, pY2)

    if long1 > long2:
        pX2 += 2**level* 256
        tX2 += 2**level

    print "calculating all quadKeys ..."
    quadKeys = allquads(tX1, tY2, tX2, tY1, level)
    print "finished calculation. "
    if len(quadKeys) <=0 :
        print 'error: quadKeys error. '
        exit()
    print "downloading subImages..."
    H = len(quadKeys)*256
    W = len(quadKeys[0])*256
    TotalNumber = H * W
    count = 0
    output = Image.new('RGB', (W, H))
    for i in range(0, len(quadKeys)):
        for j in range(0, len(quadKeys[0])):
            quadelement = quadKeys[i][j]
            request = "http://h0.ortho.tiles.virtualearth.net/tiles/h%s.jpeg?g=131&%s" % (quadelement, key)
            url = urllib.urlopen(request)
            result = url.read()
            count += 1
            if url.getcode() != 200:
                obj = json.loads(result)
                errors = obj["errorDetails"]
                for string in errors:
                    print string
            else:
                unit = Image.open(io.BytesIO(result))
                output.paste(unit, (j*256, i*256))
    print "finished download and stitching. "
    print "cropping image according to the bounding box..."
    left = pX1 - tX1 * 256
    upper = pY2 - tY2 * 256
    right = pX2 - tX1 * 256
    lower = pY1 - tY2 * 256
    print 'left %s, upper %s, right %s, lower %s' %(left, upper, right, lower)
    RealImage = output.crop((left, upper, right, lower))
    output.save("Tiles.jpeg")
    RealImage.save("Accurate scale.jpeg")
    exit()

