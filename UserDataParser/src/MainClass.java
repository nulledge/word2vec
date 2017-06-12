import java.io.*;
import java.net.*;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.io.File;
import java.io.FileWriter;
import java.io.BufferedWriter;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;


public class MainClass {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		subDirList("D:/profile/new");
	}
	
	public static void subDirList(String source){
		File dir = new File(source);
		File[] fileList = dir.listFiles();
		int peopleCount = 0;
		try{
			BufferedWriter fw = new BufferedWriter(new FileWriter("D:/SkipGram.txt", true));
			File f = new File("D:/SkipGram.txt");
			int exp = 0;
			int expCnt = 1;
			for(int i = 0 ; i < fileList.length ; i++){
				if(i > Math.pow(10, exp)) {
					System.out.println("10^" + exp++);
					expCnt = 1;
				}
				else if(i > Math.pow(10, exp-1) * expCnt) {
					System.out.println(expCnt++ + "*10^" + (exp-1));
				}
				File file = fileList[i]; 
				
				if(file.isFile()){
					// 파일이 있다면 파일 이름 출력
					//System.out.println("\t 파일 이름 = " + file.getName());
					
					List<Game> games = jsonParser (file.getAbsolutePath());
					if(games == null) continue;
					
					if(games.size() <= 1) continue;
					
					if(games.size() > 10) {
						games = games.subList(0, 5);
					}
					
					for(int index=0 ; index<games.size() ; index++) {
						fw.write(games.get(index).getName());
						fw.write("|");
						for(int cnt=0 ; cnt<games.size(); cnt++) {
							if(cnt == index) continue;
							fw.write(games.get(cnt).getName());
							if(cnt < games.size()-1) {
								if(cnt+1 == index && index == games.size()-1)
									continue;
								fw.write("^");
							}
						}
						fw.write("\n");
					}
					
				}
				else if(file.isDirectory()){
					System.out.println("디렉토리 이름 = " + file.getName());
					// 서브디렉토리가 존재하면 재귀적 방법으로 다시 탐색
					subDirList(file.getCanonicalPath().toString()); 
				}
				/*
				if(f.length()/1024/1024 > 10)
					break;
					*/
			}
			
			fw.close();
		}
		catch(IOException e){
			e.printStackTrace();
		}
		
	}
	
	public static List<Game> jsonParser (String filePath) {
		try { // parser
            JSONParser jsonParser = new JSONParser();
            //JSON데이터를 넣어 JSON Object 로 만들어 준다.
            JSONObject raw = (JSONObject) jsonParser.parse(new FileReader(filePath));
            
            JSONObject response = (JSONObject) raw.get("response");
            if(response == null) return null;
            
            JSONArray games = (JSONArray) response.get("games");
            if(games == null) return null;
            
            List<Game> gameList = new ArrayList<Game>();

            for(int i=0 ; i<games.size() ; i++) {
            	JSONObject temp = (JSONObject)games.get(i);
            	int playtime = 0;
            	String appid = "";
            	
            	if(temp.get("playtime_forever") != null)
            		playtime = Integer.parseInt(temp.get("playtime_forever").toString());
            	if(playtime < 120) continue;
            	if(temp.get("appid") != null)
            		appid = temp.get("appid").toString();
            	gameList.add(new Game(appid, playtime));
            }
            Collections.sort(gameList, new GameDescending());
            return gameList;
        }
		catch (ParseException e) {
            // TODO Auto-generated catch block
			//System.out.println(filePath);
            //e.printStackTrace();
        }
		catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return null;
	}

}

class Game {
	private String name;
	private int playTime;
	
	public Game(String name, int playTime) {
		this.name = name;
		this.playTime = playTime;
	}
	
	public String getName() {
		return name;
	}
	
	public int getPlayTime() {
		return playTime;
	}
	
	@Override
	public String toString() {
		return "(" + name + "," + playTime + ")";
	}
}

class GameDescending implements Comparator<Game> {
	
	@Override
	public int compare(Game g1, Game g2) {
		return g2.getPlayTime()-g1.getPlayTime();
	}
}