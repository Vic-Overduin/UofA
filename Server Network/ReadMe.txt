Distributed systems assignment 2:

Included files: 

Setup.java
NewsGenerator.java 
NewsAggregator.java
NewsCreator.java
NewsClient.java 

To run this system, first compile all the individual components:

Commands for compilation of all java programs:
$ javac SetUp.java
$ javac RandomArticleProducer.java
$ javac NewsAggregator.java
$ javac NewsCreator.java
$ javac Client.java


Everytime when you test the system call the Setup for cleaning out
the current directories (NA/, NC/ and C/) and setting up the environment
$ java Setup

To start the NewsAggregator, it requires one parameter for setting up the maximum number of requests from all clients.

Format: $ java NewsAggregator maximum_Number_of_requests

For a simple test, just input 1.

$ java NewsAggregator 1


You can create a NewsServer by assigning it's ID and news creation rate (NCR: in milliseconds)

Format: $ java NewsServer ID NCR

For a simple test, set ID=1 and NCR=1000 milliseconds.

$ java NewsCreator 1 1000


You can also create a NewsClient with ID to request NA for news

Format: $ java Client id

For a simple test, create a NewsClient with ID=2

$ java NewsClient 2