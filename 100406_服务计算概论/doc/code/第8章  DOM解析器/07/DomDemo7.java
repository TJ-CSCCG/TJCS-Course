import org.w3c.dom.*;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.io.File;

public class DomDemo7 {
     public static void main(String[] args) {
         int m=1;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(new File("06.xml"));

            DocumentType docType = document.getDoctype();
            NamedNodeMap entMap = docType.getEntities();

            for(int i=0;i<entMap.getLength();i++){
                Entity entNode = (Entity)entMap.item(i);
                System.out.println("实体的内容："+entNode.getTextContent());
                System.out.println("采用的编码："+entNode.getInputEncoding());
            }
            } catch (Exception e) {
            e.printStackTrace();
        }

    }



}
