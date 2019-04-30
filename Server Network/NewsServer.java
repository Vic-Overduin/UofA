import java.io.*;
import java.util.*;         //importing ArrayList
import static java.nio.file.StandardWatchEventKinds.*;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.awt.Toolkit;
import java.util.Timer;
import java.util.TimerTask;
import java.nio.file.*;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.Date;


/* News Creator:
* 1.) Created with an ID and a news creation rate (in milliseconds) both command line
*     parameters.
* 2.) Registers its ID to the aggregation server
* 3.) Based on the rate, the news nerver creates a piece of news and sends it
*     to the aggregator as plain text, in a file
*/

public class NewsServer {

    private String path;
    static private Path NA_path = Paths.get("NA");
    private int news_creation_rate;
    private int id;
    private int fileID;

    public static void main(String[] args) {

        NewsServer nc = new NewsServer(Integer.parseInt(args[0]),
        Integer.parseInt(args[1]));

        File approve = new File("NA/Approve_NC" + Integer.toString(nc.get_NC_id()));

        // If the current news server doesn't get response for the registration, it will
        // wait for 0.1sec and resends the request again
        nc.send_registration_requests();

        while (!approve.exists()) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            nc.send_registration_requests();
        }

        System.out.println("Confirmed");
        approve.delete();

        while (!nc.check_SDN()) {
            nc.next_event();
        }

        return;
    }

    NewsServer(int id, int ncr) {

        this.id = id;
        this.news_creation_rate = ncr;
        this.fileID = 1;

        // create a directory NC/
        this.path = "NC";
        File newDir = new File (this.path);
        if (!newDir.exists()) {
            try {
                newDir.mkdir();
            } catch (SecurityException se) {
                System.out.println("Cannot create Dir");
            }
        }

        // create a directory called NC_id in NC/
        this.path = "NC/NC_" + Integer.toString(id);
        newDir = new File (this.path);
        if (newDir.isDirectory() && newDir.exists()) {
            if (newDir.isDirectory()) {
                for (File c : newDir.listFiles())
                    c.delete();
            }
        } else if (!newDir.exists()) {
            try {
                newDir.mkdir();
            } catch(SecurityException se) {
                System.out.println("Cannot create Dir");
            }
        }
    }

    public void next_event() {
        try (WatchService watcher = FileSystems.getDefault().newWatchService()) {
            Map<WatchKey, Path> keyMap = new HashMap<>();
            Path path = NA_path;
            keyMap.put(path.register(watcher,
            StandardWatchEventKinds.ENTRY_CREATE),
                path);

            WatchKey watchkey;
            String next = "DONE_NC" + Integer.toString(this.id) + "_" +
            Integer.toString(this.fileID);
            boolean check = true;

            do {
                if (check) {
                    next = "DONE_NC" + Integer.toString(this.id) + "_" +
                    Integer.toString(this.fileID);

                    create_news();
                    send_news();
                    check = false;
                }

                watchkey = watcher.take();
                Path eventDir = keyMap.get(watchkey);

                for (WatchEvent<?> event : watchkey.pollEvents()) {
                    WatchEvent.Kind<?> kind = event.kind();
                    Path eventPath = (Path)event.context();

                    if (eventPath.getFileName().toString().equals(next)) {
                        check = true;
                        break;
                    }
                }

                if (check == true) {
                    File doneFile = new File("NA/" + next);
                    System.out.println("Received by NA");
                    doneFile.delete();
                    check_NA_get_news();
                    try {
                        Thread.sleep(this.news_creation_rate);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }

                    break;
                }
            } while (watchkey.reset());

        } catch(Exception e) {
            System.err.println(e);
        }
    }


    // when a news creator registers with the aggregation server, it creates a file
    // called Register_NCID
    private void send_registration_requests() {
        String registerName = "NC/Register_NC" + Integer.toString(this.id);
        createFile(registerName);
        System.out.println("Sending request to NA for registration");
    }

    // Once the news server creates a news file, it calls NewsGenerator to
    // generate a random news
    public void create_news() {
        String path = this.path + "/NC" + Integer.toString(this.id)
        + "_" + Integer.toString(this.fileID) + ".txt";
        createFile(path);

        NewsGenerator.genFile(path);
    }

    public void check_NA_get_news() {
        this.fileID++;
    }

    private void send_news() {
        String newPath = "NC/CREATOR_NC" + Integer.toString(this.id);
        createFile(newPath);
        System.out.println("Sending new news from: " + newPath);
    }

    public int get_NC_id() {
        return this.id;
    }

    public int get_file_id() {
        return this.fileID;
    }

    public int get_NC_rate() {
        return this.news_creation_rate;
    }

    private void createFile(String path) {
        File newFile;
        try {
            newFile = new File(path);
            if (!newFile.exists()) {
                newFile.createNewFile();
            }
        } catch(IOException e) {
            e.printStackTrace();
        }
    }

    public boolean check_SDN() {
        File tempF = new File("NA/SDN");
        if (tempF.exists())
            return true;
        return false;
    }
}