import org.w3c.dom.*;
import javax.xml.parsers.*;
import javax.xml.transform.*;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.dom.DOMSource;
import java.io.*;

public class xiti5{
    public static void main(String[] args) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse("xiti5.xml");

            NodeList nodeList = document.getElementsByTagName("book");
            for (int i = 0; i < nodeList.getLength(); i++) {
                Element node = (Element) nodeList.item(i);
                node.setAttribute("isbn", "x-xxx-" + i);
                Element price = document.createElement("price");
                price.appendChild(document.createTextNode("24"));
                node.appendChild(price);
            }
            NodeList titleNodeList = document.getElementsByTagName("title");
            for (int i = 0; i < titleNodeList.getLength(); i++) {
                Element titleNode = (Element) titleNodeList.item(i);
                Text tNode = (Text) titleNode.getChildNodes().item(0);
                tNode.appendData("(电子工业出版社)");
            }

            TransformerFactory tfactory =  TransformerFactory.newInstance();
            Transformer transformer = tfactory.newTransformer();
            DOMSource source = new DOMSource(document);
            FileOutputStream out = new FileOutputStream("xiti5.xml");
            StreamResult result = new StreamResult(out);
            transformer.transform(source, result);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
