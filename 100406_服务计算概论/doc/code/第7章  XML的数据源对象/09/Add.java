import org.w3c.dom.*;
import javax.xml.parsers.*;
import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import java.io.File;

public class Add {
    public static void main(String[] args) {
        boolean flag ;
        Document document = null;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            document = builder.parse(new File("02.xml"));
            document.normalize();
            Node root = document.getDocumentElement();
            if (root.hasChildNodes()) {

                /* 建立student节点 */
                Element selement = document.createElement("student");
                root.appendChild(selement);

                /*为student 添加子节点 */
                Element element = document.createElement("name");
                element.appendChild(document.createTextNode("赵六"));
                selement.appendChild(element);

                element = document.createElement("sex");
                element.appendChild(document.createTextNode("男"));
                selement.appendChild(element);

                element = document.createElement("age");
                element.appendChild(document.createTextNode("24"));
                selement.appendChild(element);

                /* 保存Document */
               flag = SaveXmlFile(document, "02.xml");
               if(flag){
                   System.out.println("成功插入数据！");
               }else{
                   System.out.println("操作失败！");
               }
            }
        }catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public static boolean SaveXmlFile(Document document, String filename) {
        boolean flag = true;
        try {
            /** 将document中的内容写入文件中  */
            TransformerFactory tFactory = TransformerFactory.newInstance();
            Transformer transformer = tFactory.newTransformer();
            /** 编码 */
            //transformer.setOutputProperty(OutputKeys.ENCODING, "GB2312");
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
