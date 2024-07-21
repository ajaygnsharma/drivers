import re

if __name__ == '__main__':
    viewOut = "all,excluded,.1,none\n\
systemview,excluded,.1,none\n\
systemview,included,.1.3.6.1.2.1.1,none\n\
systemview,included,.1.3.6.1.6.3,none\n\
systemview,included,.1.3.6.1.4.1.21369,none\n";

    viewsLine = re.split(r'\n', viewOut);

    # Don't show password
    views = {0: {'name': '0', 'flg': '0', 'subtree': '0', 'mask': '0'},
             1: {'name': '0', 'flg': '0', 'subtree': '0', 'mask': '0'},
             2: {'name': '0', 'flg': '0', 'subtree': '0', 'mask': '0'},
             3: {'name': '0', 'flg': '0', 'subtree': '0', 'mask': '0'},
             4: {'name': '0', 'flg': '0', 'subtree': '0', 'mask': '0'},
             5: {'name': '0', 'flg': '0', 'subtree': '0', 'mask': '0'},
    };

    i = 0;
    for l in viewsLine:
        token = re.split(r',', l);

        if(token[0] != ''):
            views[i] = {'name':   token[0],
                        'flg':    token[1],
                        'subtree':token[2],
                        'mask':   token[3]};

        i += 1;

    print(views);

