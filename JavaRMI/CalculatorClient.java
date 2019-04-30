package calculator;

import java.io.*;
import java.util.*;	    // needed for Stack
import java.util.Scanner;   // needed for Scanner
import java.util.Random;    // needed for Random
import java.nio.file.*;

import java.rmi.registry.Registry;
import java.rmi.registry.LocateRegistry;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

public class CalculatorClient {

    static int number_of_Threads;
    static Calculator calc;

    // Create output files	
	
    public static void createFile(String path) {
        File newFile;
        try{
            newFile = new File(path);
            if (!newFile.exists()) {
                newFile.createNewFile();
            } else {
                newFile.delete();
                newFile.createNewFile();
            }
        } catch(IOException e) {
            e.printStackTrace();
        }
    }
	

    public static void main(String[] args) {
        Random rand = new Random();
        number_of_Threads = rand.nextInt(30);

        createFile("correct_output.txt");
        createFile("RMI_output.txt");

        try {
            Registry registry = LocateRegistry.getRegistry(null);
            calc = (Calculator) registry.lookup("CalculatorServer");
        } catch (Exception e) {
            System.out.println(e);
        }

        TestClients[] clients = new TestClients[number_of_Threads];
        for (int i=0; i<number_of_Threads; i++) {
            clients[i] = new TestClients(rand.nextInt(10),i);
        }

        //Implementing threads
        Thread thread[] = new Thread[number_of_Threads];
        for(int i=0; i<number_of_Threads; i++) {
            thread[i] = new Thread(clients[i]);
        }

        try {
            for (int i=0; i<number_of_Threads; i++) {
                thread[i].start();
                thread[i].join();
            }
        } catch (Exception e) {
            System.err.println("Client exception: " + e.toString());
            e.printStackTrace();
        }
    }
}

// Creating thread by creating objects of ... class
class TestClients extends CalculatorClient implements Runnable {

    private int number_of_turns;
    private int ID;
    private String buffer_correct = "";
    private String buffer_RMI = "";

    public TestClients(int number_of_turn,int id) {
        number_of_turns = number_of_turn;
        ID = id;
    }

    @Override
    public void run() {
        // Creating the object rand of Random class
        Random rand = new Random();

        try {
            buffer_RMI += "Client" + Integer.toString(ID) + "\n";
            buffer_correct += "Client" + Integer.toString(ID) + "\n";

            // If user calls: pushValue -> pushOperator -> pop
            for (int i=0; i<number_of_turns; i++) {
                int randType = rand.nextInt(2);
                int operand1 = rand.nextInt(10000);
                int operand2 = rand.nextInt(10000);

                calc.pushValue(operand1);
                calc.pushValue(operand2);

                int operatorN = rand.nextInt(4);
                String operator;
                int result;
                switch (operatorN) {
                    case 1: operator = "+";
                            result = operand1 + operand2;
                            break;

                    case 2: operator = "-";
                            result = operand1 - operand2;
                            break;

                    case 3: operator = "*";
                            result = operand1 * operand2;
                            break;

                    case 0: operator = "/";
                            result = operand1 / operand2;
                            break;

                    default: operator = "+";
                            result = operand1 + operand2;
                            break;
                }
                calc.pushOperator(operator);

                // If randType is 0: user calls: pushValue -> pushOperator -> pop
                if (randType == 1) {
                    buffer_RMI += "The result of " + Integer.toString(operand1) + operator + Integer.toString(operand2) + " is: " + Integer.toString(calc.pop()) + "\n";
                    buffer_correct += "The result of " + Integer.toString(operand1) + operator + Integer.toString(operand2) + " is: " + Integer.toString(result) + "\n";
                }

                // If user calls: pushValue -> pushOperator -> delayPop
                else {
                    int delay = rand.nextInt(1000);
                    buffer_RMI += "The result of " + Integer.toString(operand1) + operator + Integer.toString(operand2) + " is: " + Integer.toString(calc.delayPop(delay)) + "\n";
                    buffer_correct += "The result of " + Integer.toString(operand1) + operator + Integer.toString(operand2) + " is: " + Integer.toString(result) + "\n";
                }
            }

        } catch (Exception e) {
            System.out.println(e);
        }

        write_correctOutput();
    }
	
	
	
    public void write_correctOutput() {
        FileOutputStream out = null;

        try {
            Files.write(Paths.get("RMI_output.txt"), buffer_RMI.getBytes(), StandardOpenOption.APPEND);
            Files.write(Paths.get("correct_output.txt"), buffer_correct.getBytes(), StandardOpenOption.APPEND);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
	
}
