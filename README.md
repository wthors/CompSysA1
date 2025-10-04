CompSys 2025 - Assignment A1

by William Thorsgaard

How to run:
1. Recreate the data records
    - Navigate in terminal to CompSysA1/src
    - run "make planet-latest-geonames.tsv" and wait
    - for smaller dataset now run, "head -n 20000 planet-latest_geonames.tsv > 20000records.tsv" and use the resulting file instead.
2. run "make" on the scripts/programs you want to run
3. run the program with the dataset as input, e.g. "./.id_query_naive 20000records.tsv"
4. Type one ID, or 2 coordinates seperated by a space per line.
    - "45"
    - "45 45"
5. Program runs interactively, so you can keep feeding it commands. To end, ctrl + c 
