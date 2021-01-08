import org.w3c.dom.*;
import javax.xml.parsers.*;

public class xiti4 {
    public static void main(String[] args) {
        int n = 0;
        float mathscore = 0;
        float englishscore = 0;
        float totalmath = 0;
        float totaleng = 0;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse("xiti4.xml");
            System.out.println("=====成绩单=====");
            NodeList nodeList = document.getElementsByTagName("name");
            for (int i = 0; i < nodeList.getLength(); i++) {
                Node node = nodeList.item(i);
                NodeList cNodeList = node.getChildNodes();
                System.out.print(cNodeList.item(0).getTextContent());
                for (int j = 0; j < cNodeList.getLength(); j++) {
                    Node cNode = cNodeList.item(j);
                    if (cNode.getNodeType() == Node.ELEMENT_NODE) {
                        String nodeName = cNode.getNodeName();
                        if (nodeName == "math") {
                            System.out.print(nodeName+"：");
                            mathscore = Float.parseFloat(cNode.getTextContent());
                            totalmath += mathscore;
                            System.out.println(mathscore);
                        }
                        if (nodeName == "english") {
                            System.out.print(nodeName+"：");
                            englishscore = Float.parseFloat(cNode.getTextContent());
                            totaleng += englishscore;
                            System.out.println(englishscore);
                            n++;
                            System.out.println("总分："+(mathscore+englishscore));
                        }

                    }

                }

            }
            System.out.println("=====平均分=====");
            System.out.println("math："+(totalmath/n));
            System.out.println("english："+(totaleng/n));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
