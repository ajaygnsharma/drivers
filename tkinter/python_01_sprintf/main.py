def CT0(temp=None, power=None, level=None, raw=None):
    if ((temp != None) and (power!=None) and (level!=None) and raw):
        cmd = ("%d,%d,%d,%d" % (temp, power, level, raw));
        # cmd  = str(temp)+','+str(power)+','+str(level)+','+str(raw);
        bCmd = str.encode(cmd);
        print(bCmd);



CT0(0, 0, 1, 40);

