Author: Victor Overduin (a1653894)


#Files included:

	Calculator.java

	CalculatorImplementation.java

	CalculatorServer.java

	CalculatorClient.java


#Compilation:

	Enter the following commands to properly compile:

		mkdir classes

		javac -d classes *.java


#Running:

	Enter the following commands to properly run:

		cd classes
	
#For the server:

		rmiregistry &

		java -classpath . calculator.CalculatorServer &
	
#For the client:

		java -classpath . calculator.CalculatorClient


#Usage:
	If the rmiregistry & command gives an error, there is already an instance of the registry running.
	Enter commands: ps
			kill [rmiregistry pid]
	After this proceed to run the server, omitting the rmiregistry & command.
	Server has started succesfully is the message "Server is ready." appears in the terminal.
#Ouputs:

	Two output files will be generated. correct_output.txt and RMI_output.txt

	No output will be displayed on the terminal


#Testing:

	After client finishes execution, use the following command:

		diff -w correct_output.txt RMI_output.txt

	Test is successful if no output generated.

	
	To see output generated, use the command:

		cat RMI_output.txt


#Operation:

	CalculatorClient will generate between 1 and 31 threads which each perfrom 1 to 11 requests.

	Each request contains (with equal probability)
 
		1) pushValue -> pushValue -> pushOperator -> pop

		2) pushValue -> pushValue -> pushOperator -> delayPop

	CalculatorClient will then write the result to two files:

		The correctly computed result to correct_output.txt

		The RMI computed result to RMI_output.txt


#Notes:

	Multiple threads accessing the server stack will cause a race condition and yields
 
	unexpected results due to server not regulating thread access to the server stack.
