import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.io.*;

public class DomParse {
 public DomParse(){
  try{
   DocumentBuilderFactory domfac=DocumentBuilderFactory.newInstance();
   domfac.setNamespaceAware(true);
   DocumentBuilder dombuilder=domfac.newDocumentBuilder();
   
   Document document=dombuilder.parse(new File("02.xml"));
   NodeList nodelist=document.getElementsByTagNameNS("jilin","уехЩ");
       for(int i=0;i<nodelist.getLength();i++){
          Node node=nodelist.item(i);
	  String name = node.getNodeName();
          String content=node.getTextContent();
          System.out.println(name);
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