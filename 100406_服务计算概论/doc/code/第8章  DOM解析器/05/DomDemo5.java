import org.w3c.dom.*;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.io.File;

public class DomDemo5 {
     public static void main(String[] args) {
         int m=1;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(new File("04.xml"));

            NodeList nodeList = document.getDocumentElement().getChildNodes();
            for(int i=0;i<nodeList.getLength();i++){
               Node node = nodeList.item(i);
               if(node.getNodeType()==Node.ELEMENT_NODE){
                   System.out.println(node.getNodeName());
                   NamedNodeMap attrMap = node.getAttributes();
                   for(int j=0;j<attrMap.getLength();j++){
                       Attr aNode = (Attr)attrMap.item(j);
                       System.out.print(aNode.getName()+"£º"+aNode.getValue()+"£»");
                   }
                   System.out.println(node.getTextContent());
               }
            }
            } catch (Exception e) {
            e.printStackTrace();
        }

    }



}
