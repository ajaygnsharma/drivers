from math import log

def natural_log_first_ten():
    for i in range(1,11):
        n = log(i);
        print("%d, %0.2f"%(i,n));


natural_log_first_ten();