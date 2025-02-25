#/bin/sh
echo 'include:' `find ./include -type f | xargs wc -l | tail -n 1` 'lines'
echo 'source:' `find ./source -type f | xargs wc -l | tail -n 1` 'lines'
