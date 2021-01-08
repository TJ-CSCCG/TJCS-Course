import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.io.*;


public class DomParse {
 public DomParse(){
  try{
   DocumentBuilderFactory domfac=DocumentBuilderFactory.newInstance();
   DocumentBuilder dombuilder=domfac.newDocumentBuilder();
   
   Document document=dombuilder.parse(new File("01.xml"));

     NodeList nodelist=document.getElementsByTagName("node");
     
       for(int i=0;i<nodelist.getLength();i++){
          Node node=nodelist.item(i);
          String content=node.getTextContent();
          System.out.println(content);
       }
     }catch (Exception e) {
        e.printStackTrace();
      }
    }
  
  public static void main(String[] args) {
     new DomParse();
 }
}
