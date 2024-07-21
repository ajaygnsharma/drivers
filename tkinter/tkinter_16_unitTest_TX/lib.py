
import re

# If input contains a -ve integer with length of atleast 2 digits.
# Otherwise CC4 matches 4 also.
def isNegativeNumber(line):
    expList = re.findall("-\d{2,}", line.decode("utf-8") )
    
    ok = False;
    for inputVal in expList:
        if(int(inputVal) < 0):
            ok = True;
    
    return ok;

def hasANumber(line):
    expList = re.findall("\d{2,}.\d{,}", line.decode("utf-8") )
    
    ok = False;
    for inputVal in expList:
        if(float(inputVal) > 0):
            ok = True;
    
    return ok;

def hasString(line, item):
    opList = line.decode("utf-8").split("\r\n");
    found = False;
    for opStr in opList:
        strList = opStr.split("=");
        if(len(strList) > 0):
            for myStr in strList:
                if(item in myStr):
                    found = True;
                    
    return found;