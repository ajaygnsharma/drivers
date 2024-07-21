* resistor connected between 1,2
r1 1 2 1k

* cap connected between 2 and 0
c1 2 0 1u

*piecewise linear input voltage between 1, 0
vin 1 0 pwl (0 0 10ms 0 11ms 5v 20ms 5v)

.tran .02ms 20ms

.control
run
plot v(1) v(2)
.endc
.end
