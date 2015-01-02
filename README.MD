About
=====================================
**Distributed Mutual Exclusion Algorithm (September 2013 - December 2013)**  
*A quorum-based and token-based mutual exclusion algorithm in a distributed system*
 - Applied TCP Socket programming in C on Linux
 - Practiced Linux commands and tools
 - Used JIRA for project management

Note
=====================================
This was a team project in my telecom software design course  
Not everyone in our team uses git, so I only checked in the codes we submitted  
**The project cannot be run anymore because it can only run on servers in my school**

How to run and Trouble Shooting
=========================
```
login to: cs1, net44, net01~20
ssh yourNetid@netXX.utdallas.edu

1.(direct cs1 & netXX to the project folder)
cd AOS/team2/

2.(at cs1, compile everything)
make n
make ls
make sw
make tb

3. edit config.txt

4. (at net01~net20, start listening server)
./listenserver.out

5.(at net44, start testbed)
./testbed

(wait for results...)

6. after 2 algorithms finish (when you see "RECEIVE END SIGNAL" on net01~20  for the second time except 05,10,15,20), testbed will wait 20 sec before computing results, during this time:

6.1 press Ctrl+C on net01~net20 to shutdown listenserver
6.2 (for net05 net10 net15 net20 you need to kill listenserver.out for unknown reason)
killall ./listenserver.out
(or)
bash KALL

7. show result on testbed(net44), and open output1 and output2 to show critical sections to TA

8. edit config.txt for next configuration, back to step 4


truoble shooting:
1.If testbed said
Contacting listening server XX failed
then it could be that you forget to start listening server or processes aren't shutdowned properly on netXX

2. If the program hangs and you press Ctrl+C to force close everything, run
bash KALL
on every netXX machine to shutdown processes properly
(I've tested and confirmed that killProcs doesn't work)

3. If they start to lose messages, edit ip.txt and use two complete different ports and re-run program
```