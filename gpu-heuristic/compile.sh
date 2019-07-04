#!/bin/bash

(cd build; cmake ../; make; mv HEA ../bin/)

# If everything is ok, the exec file will be written on folder .bin/