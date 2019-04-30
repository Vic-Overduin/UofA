import java.io.*;
import java.util.*;         
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

/* NewsNewsClient:
* 1.) created with an ID received as command line parameter
* 2.) sends a request for news from the server
* 3.) once news received from the server, prints the news out and shuts down
*/
public class NewsClient {

    private ArrayList<File> news = new ArrayList<>();
    private String path;
    private String responsePath;
    private Path NA_dir = Paths.get("NA");
    private int id;

    public static void main(String[] args) {
        NewsClient c = new NewsClient(Integer.parseInt(args[0]));
        File responseCID = new File("NA/Response_C" +
        Integer.toString(c.get_C_id()));
        c.receive_news();


        if (responseCID.exists()) {
            c.print_file();
            responseCID.delete();
            c.shutdown();
        }
    }

    NewsClient(int id) {
        this.id = id;

        // create a directory NC/
        this.path = "C";
        this.responsePath = "NA/Response_C" + Integer.toString(id);
        File newDir = new File (this.path);
        if (!newDir.exists()) {
            try {
                newDir.mkdir();
            } catch (SecurityException se) {
                System.out.println("Cannot create Dir");
            }
        }

        // create a directory called C_id in C/
        this.path = "C/C_" + Integer.toString(id);
        newDir = new File (this.path);
        if (newDir.isDirectory() && newDir.exists()) {
            if (newDir.isDirectory()) {
                for (File c : newDir.listFiles())
                    c.delete();
            }
        } else if (!newDir.exists()) {
            try {
                newDir.mkdir();
            } catch (SecurityException se) {
                System.out.println("Cannot create Dir");
            }
        }
    }

    public void send_request() {
        String requestName = "C/NewsClient_C" + Integer.toString(this.id);
        createFile(requestName);
    }

	//handling the receiving of the news stream
    public void receive_news() {
        boolean goNext = false;
        File tempF = new File(this.responsePath);

        try (WatchService watcher = FileSystems.getDefault().newWatchService()) {
            Map<WatchKey, Path> keyMap = new HashMap<>();
            Path path = Paths.get(this.path);
            keyMap.put(path.register(watcher,
                StandardWatchEventKinds.ENTRY_CREATE),
                path);

            keyMap.put(this.NA_dir.register(watcher,
                StandardWatchEventKinds.ENTRY_CREATE),
                this.NA_dir);


            WatchKey watchkey;
            send_request();
            do {
                watchkey = watcher.take();
                Path eventDir = keyMap.get(watchkey);

                for (WatchEvent<?> event : watchkey.pollEvents()) {
                    WatchEvent.Kind<?> kind = event.kind();
                    Path eventPath = (Path)event.context();
                    File file = new File(this.path + "/" +
                    eventPath.toString());

                    if (eventPath.toString().equals("NA/Response_C" +
                    Integer.toString(this.id))) {
                        goNext = true;
                        break;
                    } else if (eventPath.toString().startsWith("NC_")) {
                        news.add(file);
                    }
                }

                if (goNext == true) {
                    break;
                }
            } while (watchkey.reset() && !tempF.exists());

        } catch(Exception e) {
            System.err.println(e);
        }
    }
	
	//prints all the news items in order
    public void print_file() {
        String order = "";

        for (int i = 0; i < news.size() - 1; i++) {
            File file = news.get(i);
            order += file.getName() + "\n";
            System.out.println(file.getName());

            try {
                BufferedReader br = new BufferedReader(new FileReader (file));
                String line = null;
                while ((line = br.readLine()) != null) {
                    System.out.println(line);
                }
            } catch(IOException e) {
                e.printStackTrace();
            }
        }

        System.out.println();
        System.out.println("This is the news in order:");
        System.out.println(order);
        System.out.println();
    }

    private void shutdown() {
        System.out.println("C" + Integer.toString(this.id) +
        " is shutting down");
    }


    public int get_C_id(){
        return this.id;
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
}