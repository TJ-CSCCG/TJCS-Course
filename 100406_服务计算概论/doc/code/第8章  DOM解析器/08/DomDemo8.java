import org.w3c.dom.*;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.io.File;

public class DomDemo8 {
     public static void main(String[] args) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setIgnoringElementContentWhitespace(true);
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(new File("07.xml"));

           Element root = document.getDocumentElement();
            NodeList nodeList = root.getChildNodes();
            display(nodeList);
            } catch (Exception e) {
            e.printStackTrace();
        }

    }
    public static void display(NodeList nodeList) {
        for (int i = 0; i < nodeList.getLength(); i++) {
            Node node = nodeList.item(i);
            if (node.getNodeType() == Node.TEXT_NODE) {
                Text tNode = (Text) node;
                String content = tNode.getWholeText();
                System.out.println(content);

            }
            if (node.getNodeType() == Node.ELEMENT_NODE) {
                Element eNode = (Element) node;
                System.out.print(eNode.getNodeName() + "£º");
                NodeList cNodeList = eNode.getChildNodes();
                display(cNodeList);
            }
        }
    }
}
