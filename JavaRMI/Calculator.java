package calculator;

import java.io.*;
import java.util.Scanner;		//needed for Scanner
import java.util.Random;		//needed for Random
import java.math.BigInteger;		//needed for BigInteger
import java.rmi.Remote;
import java.rmi.RemoteException;


public interface Calculator extends Remote {

    // Push operand to calculator stack.
    public void pushValue(int operand) throws RemoteException;

    // Push an operator ("+","-","*","/") to the stack,
    // then pop two operands at top of stack,
    // then apply the operation and push the result back on the stack.
    public void pushOperator(String operator) throws RemoteException;

    // Pop the top of the calculator stack and return it to the client.
    public int pop() throws RemoteException;

    // Check if the stack is empty, false otherwise.
    public boolean isEmpty() throws RemoteException;

    // Waits ms milliseconds before executing the pop operation.
    public int delayPop(int ms) throws RemoteException;

}
