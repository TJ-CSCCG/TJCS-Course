import org.w3c.dom.*;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.io.File;

public class DomDemo3 {
    static int textNode = 0, elementNode = 0;

    public static void main(String[] args) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(new File("01.xml"));

            Element root = document.getDocumentElement();
            System.out.print(root.getNodeName());

            NodeList nodeList = root.getChildNodes();
            display(nodeList);
            System.out.println("文件01.xml中共有：");
            System.out.println("Element类型节点：" + elementNode + "个");
            System.out.println("Text类型节点：" + textNode + "个");
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    public static void display(NodeList nodeList) {
        for (int i = 0; i < nodeList.getLength(); i++) {
            Node node = nodeList.item(i);
            if (node.getNodeType() == Node.TEXT_NODE) {
                Text tNode = (Text) node;
                textNode++;
                String content = tNode.getWholeText();
                System.out.print(content);

            }
            if (node.getNodeType() == Node.ELEMENT_NODE) {
                Element eNode = (Element) node;
                elementNode++;
                System.out.print(eNode.getNodeName() + "：");
                NodeList cNodeList = eNode.getChildNodes();
                display(cNodeList);
            }
        }
    }
}
