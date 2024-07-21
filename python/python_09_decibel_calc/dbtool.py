#!/home/asharma/miniconda3/bin/python
import numpy as np
import tkinter as tk
import getopt
import sys
import math

# Relative power
def to_dB(arr, ref):
    if arr != 0:
        p = 20 * np.log10(abs(arr) / ref)
        return round(p);

    else:
        return -60

# Absolute power
def to_dBm(arr):
    ref = 1
    if arr != 0:
        arr = arr * 1000;
        p = 10 * np.log10(abs(arr) / ref);
        return round(p);

    else:
        return -60

def to_mBm(arr):
    ref = 1
    if arr != 0:
        arr = arr * 1000;
        p = 10 * np.log10(abs(arr) / ref);
        p = p * 100; ''' Convert to mbm from dbm '''

        p = int(round(p/10.0)) * 10;
        return round(p);

    else:
        return -60


def to_mW(arr):
    if arr != 0:
        p = 10 ** (abs(arr)/10)
        return round(p);

def to_W(arr):
    if arr != 0:
        p = (10 ** (abs(arr)/10))/1000;
        return round(p);

'''
print(to_dB(125));
print(to_dBm(125));

print(to_mW(51));
print(to_W(51));

print(to_mW(-55));
print(to_W(-55));

print(to_mW(-20));
print(to_W(-20));
'''

def to_dB_g():
    pass;
    '''watts = int(entry_W.get());
    r = to_dB(watts);
    lblResult["text"] = str(r);'''

def gui():
    window = tk.Tk();

    window.rowconfigure(0, minsize=100);
    window.columnconfigure([0, 1, 2], minsize=50);

    frame1 = tk.Frame(master=window);
    entry_W = tk.Entry(master=frame1, width=10);
    lbl_W = tk.Label(master=frame1, text="Watts(W)");

    entry_W.grid(row=0, column=0, stick="e");
    lbl_W.grid(row=0, column=1, sticky="w");


    btnConvert = tk.Button(master=window, text="convert", command=to_dB_g);
    lblResult  = tk.Label(master=window, text="dB");


    frame1.grid(row=0,column=0, padx=10);
    btnConvert.grid(row=0, column=1, pady=10);
    lblResult.grid(row=0, column=2, padx=10);

    window.mainloop();

def usage():
    print("dbtool --w2db=<Watts>");
    print("dbtool --w2db=<Watts> --ref=<ref>");
    print("dbtool --w2dbm=<Watts>");
    print("dbtool --w2mbm=<Watts>");
    print("dbtool --dbm2mw=<dbm>");
    print("dbtool --dbm2w=<dbm>");


def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "b:",
                                   ["band","help"])
    except getopt.GetoptError as err:
        # print help information and exit:
        print(err)  # will print something like "option -a not recognized"
        usage();
        sys.exit(2)

    _w2db=0;
    ref=0;
    p = 0;

    for o, a in opts: # o is the name of the option and a is its value
        if o in ("-a", "--w2db"):
            _w2db = 1;
            p = a;
        elif o in ("-b", "--w2dbm"):
            print(to_dBm(int(a)));
            sys.exit();
        elif o in ("-m", "--w2mbm"):
            print(to_mBm(int(a)));
            sys.exit();
        elif o in ("-c", "--dbm2mw"):
            print(to_mW(int(a)));
            sys.exit();
        elif o in ("-d", "--dbm2w"):
            print(to_W(int(a)));
            sys.exit();
        elif o in ("-h","--help"):
            usage();
            sys.exit();
        elif o in ("-r","--ref"):
            ref=a;
        else:
            assert False, "unhandled option"

    if(_w2db):
        if(ref != 0):
            print( to_dB(int(p), float(ref)) );
        else:
            print( to_dB(int(p),1) );

        sys.exit();

    pass;

if __name__ == '__main__':
    main();
