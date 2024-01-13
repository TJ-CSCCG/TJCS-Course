import org.w3c.dom.*;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.io.File;

public class DomDemo2 {
    public static void main(String[] args) {
        try{
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(new File("02.xml"));

            Element root = document.getDocumentElement();
            System.out.println(root.getNodeName());

            NodeList nodelist = root.getChildNodes();
            for(int i=0;i<nodelist.getLength();i++){
                Node node = nodelist.item(i);
                if(node.getNodeType() == Node.ELEMENT_NODE){
                    Element enode = (Element)node;
                    System.out.print(enode.getTagName());
                    if(enode.hasAttribute("from")){
                    System.out.println("£¨"+enode.getAttribute("from")+"£©");
                    }else{
                        System.out.println();
                    }
                    System.out.println(enode.getTextContent());
                }
            }
        }catch(Exception e){
            e.printStackTrace();
        }

    }
}
