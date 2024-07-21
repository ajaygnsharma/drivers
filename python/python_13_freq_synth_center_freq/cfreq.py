#!/home/asharma/miniconda3/bin/python
import getopt
import sys


Frequency_table = [
    ('STD_C', 5850, 6425, 950, 1525 ),
    ('PALAPA', 5850, 6425, 950, 1525 )
];

def print_center_frequency(index):
    delta = (Frequency_table[index][4] - Frequency_table[index][3]) / 2;
    cFreq = Frequency_table[index][3] + delta;

    print("Band: %s"%(Frequency_table[index][0]));
    print("RF Min=%d"%(Frequency_table[index][1]));
    print("RF Max=%d" % (Frequency_table[index][2]));
    print("L Band Min=%d" % (Frequency_table[index][3]));
    print("L Band Max=%d" % (Frequency_table[index][4]));
    print("L Band Median=%d" % cFreq);

def center_frequency(band):
    Band  = band.upper();
    delta = 0;
    cFreq = 0;
    index = 0;

    match Band:
        case "STD C":
            index = 0;
            print_center_frequency(index);
            return 0;

        case "PALAPA":
            index = 1;
            print_center_frequency(index);
            return 0;

        case _:
            print("ERROR: wrong band %s"%(Band));


def usage():
    print("cfreq --band=<std c|palapa>");

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "b:h",
                                   ["band=","help"])
    except getopt.GetoptError as err:
        # print help information and exit:
        print(err)  # will print something like "option -a not recognized"
        usage();
        sys.exit(2)

    band = "";

    for o, a in opts: # o is the name of the option and a is its value
        if o in ("-b", "--band"):
            band = a;
            center_frequency(band);
            sys.exit();
        elif o in ("-h","--help"):
            usage();
            sys.exit();
        else:
            assert False, "unhandled option"

    pass;

if __name__ == '__main__':
    main();
