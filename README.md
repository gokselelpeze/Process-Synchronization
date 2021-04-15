# Process-Synchronization
Operating Systems Homework

# Assignment#2 - Covid-19 Test Unit

# Goal
In this assignment, you will solve the synchronization problem using semaphores and mutex. You
should suffer from deadlock and/or starvation. You are expected to simulate and solve the
“COVID-19 Test Unit” synchronization problem, which its details are given.

# Implementation Details & Requirements
Today, the units that make Covid-19 tests in medical centers have a very important and critical role.
With the increase in cases and patients, this may cause the density in these units after fillation studies.
Before the test is applied, people are waiting for a long time at the entrance of the hospitals. Because
there is a limited number of test units and healthcare staff. We are launching the “Covid-19 Test Unit”
application to facilitate this. This will prevent both people from creating long queues and to lock the
waiting rooms before being full. Also it will provide an efficient and fair sharing because units will
contain people up to its capacity<br>

With your simulation and solution with mutex and semaphores students hopefully will no longer wait
so long for working.<br>
● Each waiting room has 3 people.<br>
● People will come to the unit in continuous and random periods.<br>
● The hospital has 8 units and also 8 healthcare staff. So, each unit contains one staff.<br>
● The states for each unit:<br>
&#9;○ Entry free state: Staff will announce to the people his remaining places to get in the
&#9;waiting room, if it has one or more people in the room. For example, each room has 2
&#9;people and waits for one more person and the staff calls “The last people, let's start!
&#9;Please, pay attention to your social distance and hygiene; use a mask.”<br>
&#9;○ Idle (Empty) state: If there are no people in the room, the staff will ventilate the room.
&#9;Being idle is forbidden for the staff. If any people came, they should open the room.
&#9;Don't forget that no people are waiting.<br>
&#9;○ Full and busy state: If there are 3 people in the room, the room will be in a busy state.<br>
● The states for each people:<br>
&#9;○ Waiting in room: When a person comes at room, she/he gets in the nearly full
&#9;capacity room. If there are no people in the room, the first person alerts the staff if he
&#9;is ventilating. And the people prepare for the test and fill out the form until the
&#9;room’s full. After the room is full, the test process will be applied to them and
&#9;together they empty the room.<br>
&#9;○ Waiting at hospital: If there is no empty room, she/he's gonna wait for a room at the
&#9;outdoor waiting hole of the hospital.


![Screen Shot 2021-01-09 at 23 00 04](https://user-images.githubusercontent.com/47809642/114874074-a0d05e00-9e04-11eb-94de-b8bfdb69b0fd.png)

