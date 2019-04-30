package calculator;

import java.io.*;
import java.util.*;         //Needed for Stack

import java.rmi.server.UnicastRemoteObject;
import java.rmi.Remote;
import java.rmi.RemoteException;


public class CalculatorImplementation implements Calculator {

    // Push operand to calculator stack.
    @Override
    public synchronized void pushValue(int operand) {
        stack.push(new Integer(operand));
    }
	
    // Push an operator ("+","-","*","/") to the stack,
    // then pop two operands at top of stack,
    // then apply the operation and push the result back on the stack.
    @Override
    public void pushOperator(String operator) throws UnsupportedOperationException, IllegalStateException {

        if (stack.empty()) throw new IllegalStateException();
        int operand2 = stack.pop();
        if (stack.empty()) throw new IllegalStateException();
        int operand1 = stack.pop();

        if (operator.equals("+")) {
            stack.push(operand1 + operand2);
        } else if (operator.equals("-")) {
            stack.push(operand1 - operand2);
        } else if (operator.equals("*")) {
            stack.push(operand1 * operand2);
        } else if (operator.equals("/")) {
            stack.push(operand1 / operand2);
        } else {
            // If the operator is not supported, throw UnsupportedOperationException
            throw new UnsupportedOperationException();
        }

    }

    // Pop the top of the calculator stack and return it to the client.
    @Override
    public synchronized int pop() {
        return stack.pop();
    }


    // Check if the stack is empty, false otherwise.
    @Override
    public boolean isEmpty() {
        return stack.empty();
    }

    // Waits ms milliseconds before executing the pop operation.
    @Override
    public synchronized int delayPop(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        return stack.pop();
    }

    private Stack<Integer> stack = new Stack<Integer>();

}
