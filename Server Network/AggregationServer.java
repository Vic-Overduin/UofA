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


/* News Aggregator:
*  1.) responds to a maximum number of requests from all clients
*      (command line parameter), then shuts down.
*  2.) responds to registration requests from news creators
*  3.) receives news from many news creators, and orders them
*  4.) sends news items to any client, only upon request
*  5.) once the maximum number of requests has been reached, notifies the news
*      creators through a Shut Down Now message and shuts down
*  6.) the news aggregator must have a dedicated news channel for each of its
*      registered news creators
*  7.) writes a log file
*/
public class NewsAggregator {

    static ArrayList<Integer> NewsServerID = new ArrayList<>();
    static ArrayList<Integer> ClientID = new ArrayList<>();
    static ArrayList<File> news = new ArrayList<>();
    static String log_buffer = "";
    static Path nc_dir = Paths.get("NC");
    static Path c_dir = Paths.get("C");
    static Path na_news = Paths.get("NA/News");
    static int maximum_request;
    static int current_requests = 0;

    public static void main(String[] args) {
        NewsAggregator na = new NewsAggregator(Integer.parseInt(args[0]));

        while (na.check_num_requests()) {
            na.check_Next_Event();
        }
        try {
            Thread.sleep(6 * (Integer) news.size());
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        na.write_logs();

        System.exit(0);
    }

    public NewsAggregator(int max) {

        maximum_request = max;
        String path = "NA";
        File newDir = new File(path);
        if (!newDir.exists()) {
            try {
                newDir.mkdir();
            } catch(SecurityException se) {
                System.out.println("Cannot create Dir");
            }
        }

        path = "NA/News";
        newDir = new File(path);
        if (!newDir.exists()) {
            try {
                newDir.mkdir();
            } catch(SecurityException se) {
                System.out.println("Cannot create Dir");
            }
        }
    }

    public void check_Next_Event() {
        boolean haveNewsServer = false;
        boolean haveClient = false;
        int next_CNID = -1;
        int next_CID = -1;

        try (WatchService watcher = FileSystems.getDefault().newWatchService()) {
            Map<WatchKey, Path> keyMap = new HashMap<>();
            Path path_NC = nc_dir;
            keyMap.put(path_NC.register(watcher,
                StandardWatchEventKinds.ENTRY_CREATE),
                path_NC);

            Path path_C = c_dir;
            keyMap.put(path_C.register(watcher,
                StandardWatchEventKinds.ENTRY_CREATE),
                path_C);

            WatchKey watchkey;

            do{
                next_CNID = -1;
                next_CID = -1;
                haveNewsServer = false;
                haveClient = false;
                watchkey = watcher.take();

                Path eventDir = keyMap.get(watchkey);

                for (WatchEvent<?> event : watchkey.pollEvents()) {
                    WatchEvent.Kind<?> kind = event.kind();
                    Path eventPath = (Path) event.context();


                    if (eventPath.getFileName().toString().
                    startsWith("Register_NC")) {

                        haveNewsServer = true;
                        next_CNID = Integer.parseInt(eventPath.getFileName()
                        .toString().substring(11));
                        break;
                    } else if (eventPath.getFileName().toString().
                    startsWith("Client_C")) {
                        haveClient = true;
                        next_CID = Integer.parseInt(eventPath.getFileName().
                        toString().substring(8));
                        break;
                    }

                }

                if (haveNewsServer == true) {
                    response_NC_requests(next_CNID);
                    NAReceiver nar = new NAReceiver(maximum_request, next_CNID);
                    NAReceivingNews narec = new NAReceivingNews(nar, next_CNID);
                    Thread th = new Thread(narec);
                    th.start();
                    break;
                } else if (haveClient == true) {
                    NASender nas = new NASender(maximum_request, next_CID);
                    NASendingNews nasend = new NASendingNews(nas, next_CID);
                    Thread th = new Thread(nasend);
                    th.start();
                    break;
                }


            } while (watchkey.reset());

        } catch(Exception e) {
            System.err.println(e);
        }
    }

    /* This part is for responding to requests from news creators */
    public void response_NC_requests(int id) throws SecurityException {
        int size = NewsServerID.size();
        File registerFile;

        int i = check_if_NC_registrated(id);
        if (i >= 0) {
            System.out.println("This NC has already been registered.");
        } else {
            System.out.println("Register new NC ID: " + id);
            log_buffer += "NA -NEW CREATOR- "+ Integer.toString(id) + "\n";
            log_buffer += "NA -CONFIRM- "+ Integer.toString(id) + "\n";

            NewsServerID.add(id);

            String registerName = "NA/Approve_NC" + Integer.toString(id);
            createFile(registerName);
        }
    }

    // If the id is already registered, it will return the index in NewsServerID
    // Otherwise, return -1.
    public int check_if_NC_registrated(int id) {
        int size = NewsServerID.size();

        for (int i = 0; i < size; i++) {
            if (NewsServerID.get(i) == id) {
                return i;
            }
        }
        return -1;
    }

    public void write_logs() {
        FileOutputStream out = null;

        File logfile;
        log_buffer = "Log File:\n" + log_buffer;
        log_buffer += "NA -SDN-";
        System.out.println("Shut Down Now");

        try {
            logfile = new File("NA/logfile.txt");
            out = new FileOutputStream(logfile);

            if (!logfile.exists()) {
                logfile.createNewFile();
            }

            byte[] contenInBytes = log_buffer.getBytes();

            out.write(contenInBytes);
            out.flush();
            out.close();

            System.out.println("End of logfile");

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (out != null) {
                    out.close();
                }
            } catch(IOException e) {
                e.printStackTrace();
            }
        }
    }

    public void createFile(String path) {
        File newFile;
        try{
            newFile = new File(path);
            if (!newFile.exists()) {
                newFile.createNewFile();
            }
        } catch(IOException e) {
            e.printStackTrace();
        }
    }

    public boolean check_num_requests() {
        if (current_requests < maximum_request){
            return true;
        }

        createFile("NA/SDN");
        return false;
    }
}

// Receiver class for handling news creator requests
class NAReceiver extends NewsAggregator {

    private int nc_id;
    private int fileid;
    private Path nc_path;

    public NAReceiver(int max, int nc_id) {
        super(max);
        this.nc_id = nc_id;
        this.nc_path = Paths.get("NC/NC_" + Integer.toString(this.nc_id) + "/");
        this.fileid = 1;
    }

    public void get_news(File file) throws IOException {
        FileReader fr = null;
        FileWriter fw = null;
        File destNews;

        int i = check_if_NC_registrated(this.nc_id);

        if(i >= 0){
            log_buffer += "NA -RECEIVED- news from NC_"+ Integer.
            toString(this.nc_id) + "\n";

            System.out.println("NA -RECEIVED- news from NC_"+ Integer.
            toString(this.nc_id));

            destNews = new File ("NA/News/NC_" + Integer.toString(this.nc_id) +
            "_" + Integer.toString(this.fileid));

            try {
                fr = new FileReader(file);
                fw = new FileWriter(destNews);
                int c;
                while ((c = fr.read()) != -1)
                    fw.write(c);
            } finally {
                if (fr != null) {
                    fr.close();
                }
                if (fw != null) {
                    fw.close();
                }
            }
            order_news(destNews);
            send_msg_back();
            this.fileid++;
        }
    }

    public void order_news(File file) {
        news.add(file);
    }

    public void send_msg_back() {
        String fileName = "NA/DONE_NC" + Integer.toString(this.nc_id) +  "_" +
        Integer.toString(this.fileid);
        System.out.println(fileName);
        createFile(fileName);
    }
}

// Thread class that receives new news from the creator threads
class NAReceivingNews implements Runnable {
    private NAReceiver nar;
    private int nc_id;
    private int nc_file_id;

    public NAReceivingNews(NAReceiver nar, int nc_id){
        this.nar = nar;
        this.nc_id = nc_id;
        nc_file_id = 1;
    }

    @Override
    public void run() {
        File sdn_file = new File("NA/SDN");
        try (WatchService watcher = FileSystems.getDefault().newWatchService()) {
            Map<WatchKey, Path> keyMap = new HashMap<>();
            Path path = Paths.get("NC/NC_" + Integer.toString(this.nc_id));
            keyMap.put(path.register(watcher,
                StandardWatchEventKinds.ENTRY_CREATE),
                path);

            WatchKey watchkey;
            do{
                watchkey = watcher.take();
                Path eventDir = keyMap.get(watchkey);

                for (WatchEvent<?> event : watchkey.pollEvents()) {
                    WatchEvent.Kind<?> kind = event.kind();
                    Path eventPath = (Path)event.context();

                    File temp = new File("NC/NC_" + Integer.toString(this.nc_id)
                    + "/" + eventPath.toString());

                    Thread.sleep(10);
                    nar.get_news(temp);

                    if (sdn_file.exists()) {
                        return;
                    }
                }
            } while (watchkey.reset());

        } catch(Exception e) {
            System.err.println(e);
        }
    }
}

// NASender class for handling client requests
class NASender extends NewsAggregator {
    private int c_id;
    private Path c_path;
    private ArrayList<File> news_NAS = new ArrayList<>();

    public NASender(int max, int c_id) {
        super(max);
        this.c_id = c_id;
        current_requests++;
    }

    /*Responding to requests from clients and writing requests into the log*/
    public void sendNews_to_Client(int C_ID) throws IOException {
        log_buffer += "NA -REQUEST- from Client ID: " + Integer.toString(C_ID)
        + "\n";

        System.out.println("NA -REQUEST- from Client ID: " + Integer.toString(C_ID));
        news_NAS = news;
        String path ="C/C_" + Integer.toString(C_ID) + "/" ;
        FileReader fr = null;
        FileWriter fw = null;
        String destP;
        File newsPath = new File(na_news.toString());
        if (newsPath.isDirectory() && newsPath.exists()) {
            for (int i = 0; i < news.size(); i++) {
                try {
                    int temp;
                    fr = new FileReader(news.get(i));
                    destP = path + news.get(i).getName();
                    fw = new FileWriter(destP);

                    while ((temp = fr.read()) != -1)
                        fw.write(temp);
                } finally {
                    if (fr != null) {
                        fr.close();
                    }
                    if(fw != null) {
                        fw.close();
                    }
                }
            }
        }
        response_to_Client(C_ID);
        return;
    }

    public void response_to_Client(int id) {
        String responseFile = "NA/Response_C" + Integer.toString(id);
        createFile(responseFile);
        log_buffer += "NA -SENT- to Client_" + Integer.toString(id) + "\n";
        System.out.println("NA -SENT- to Client_" + Integer.toString(id));
    }
}

// Thread that sends news to the clients
class NASendingNews implements Runnable {
    private NASender nas;
    private int c_id;
    public NASendingNews(NASender nas, int c_id) {
        this.nas = nas;
        this.c_id = c_id;
    }

    @Override
    public void run() {
        try {
            this.nas.sendNews_to_Client(this.c_id);
        } catch(IOException e) {
            e.printStackTrace();
        }
        return;
    }
}