import java.io.*;
import java.util.*;       
import java.io.File;
import java.io.IOException;
import java.nio.file.*;


/* Setup:
* This is the setup, which is to be called between each run of the system.
* Cleans out all the directories associated with the clients, aggregator and news servers
*/

public class Setup {

    public static void main(String[] args) {

        System.out.println("Clean up all the directory");

        File file = new File("NA");
        deleteDirectory(file);
        file.mkdir();
        file = new File("NC");
        deleteDirectory(file);
        file.mkdir();
        file = new File("C");
        deleteDirectory(file);
        file.mkdir();
    }

    public static boolean deleteDirectory(File directoryToBeDeleted) {
        File[] allContents = directoryToBeDeleted.listFiles();
        if (allContents != null) {
            for (File file : allContents) {
                deleteDirectory(file);
            }
        }
        return directoryToBeDeleted.delete();
    }

}