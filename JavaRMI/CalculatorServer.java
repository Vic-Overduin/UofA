package calculator;

import java.io.*;
import java.util.*;         //needed for Stack

import java.rmi.AlreadyBoundException;
import java.rmi.NoSuchObjectException;
import java.rmi.RemoteException;
import java.rmi.registry.Registry;
import java.rmi.registry.LocateRegistry;
import java.rmi.server.UnicastRemoteObject;

public class CalculatorServer {

    private final Calculator serverCalc;
    private final String serverName;
    private final int severPort;

    private Registry serverReg;

    public CalculatorServer(Calculator calc, String name, int port) {
        serverCalc = calc;
        serverName = name;
        severPort = port;
    }

    public void runServer() throws RemoteException, AlreadyBoundException {
        Calculator stub = (Calculator) UnicastRemoteObject.exportObject(serverCalc,0);

        // Bind the remote object's stub in the registry
        serverReg = LocateRegistry.createRegistry(severPort);
        Registry registry = LocateRegistry.getRegistry();
        registry.rebind(serverName, stub);
    }

    public void stopServer() throws NoSuchObjectException {
        UnicastRemoteObject.unexportObject(serverCalc,true);
        UnicastRemoteObject.unexportObject(serverReg,true);
    }

    public static void main(String args[]) {

        CalculatorImplementation calc = new CalculatorImplementation();
        CalculatorServer server = new CalculatorServer(calc,"CalculatorServer",1099);
        try {
            server.runServer();
            System.err.println("Server ready");
        } catch (Exception e) {
            System.err.println("Server exception: " + e.toString());
            e.printStackTrace();
        }
    }
 }
