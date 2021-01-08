import org.w3c.dom.*;
import javax.xml.transform.*;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.dom.DOMSource;
import javax.xml.parsers.*;
import java.io.File;

public class UpdateAndDelete {
    public static void main(String[] args) {
        Boolean flag;
        Document document = null;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            document = builder.parse(new File("02.xml"));
            document.normalize();
            Node root = document.getDocumentElement();
            /* 如果root有子元素 */
            if (root.hasChildNodes()) {
                /*获得student节点列表 */
                NodeList studentnodes = root.getChildNodes();
                /* 循环取得student所有节点 */
                for (int i = 0; i < studentnodes.getLength(); i++) {
                    NodeList studentlist = studentnodes.item(i).getChildNodes();
                    for (int k = 0; k < studentlist.getLength(); k++) {
                        Node subnode = studentlist.item(k);
                        /* 修改name值 */
                        if (subnode.getNodeType() == Node.ELEMENT_NODE && subnode.getFirstChild().getNodeValue().equals("赵六"))
                        {
                            subnode.getFirstChild().setNodeValue("赵七");
                        }
                         /* 删除节点 */
                        if (subnode.getNodeType() == Node.ELEMENT_NODE && subnode.getFirstChild().getNodeValue().equals("王五")) {
                            root.removeChild(studentnodes.item(i));
                        }
                    }
                }
            }

            /* 修改完后保存 */
            flag = SaveXmlFile(document, "02.xml");
            if (flag) {
                System.out.println("操作成功！");
            } else {
                System.out.println("操作失败！");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static boolean SaveXmlFile(Document document, String filename) {
        boolean flag = true;
        try {
            /* 将document中的内容写入文件中  */
            TransformerFactory tFactory = TransformerFactory.newInstance();
            Transformer transformer = tFactory.newTransformer();
            DOMSource source = new DOMSource(document);
            StreamResult result = new StreamResult(new File(filename));
            transformer.transform(source, result);
        } catch (Exception ex) {
            flag = false;
            ex.printStackTrace();
        }
        return flag;
    }
}
