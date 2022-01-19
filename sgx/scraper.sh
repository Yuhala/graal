#!/bin/bash

#My vim hints

:g!/\<\(Num\|Read\)\>/d


#Search file and remove all lines that do not start with Num or Read
:%s/^[^Num|Read].*\n//

