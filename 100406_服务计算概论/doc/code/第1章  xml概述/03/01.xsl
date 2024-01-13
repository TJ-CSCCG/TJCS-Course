<?xml version="1.0" ?>
<xsl:stylesheet version="1.0"
 xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:template match="/">

<html>
	<body>
		<center>
		<table border="1" width="300" >
		<tr>
			<td hight="15" align="center">nmae</td>
			<td hight="15" align="center">sex</td>
			<td hight="15" align="center">birthday</td>
		</tr>
		<xsl:for-each select="persions/persion">
		<tr>
			<td hight="15" align="center"><xsl:value-of select="name" /></td>
			<td hight="15" align="center"><xsl:value-of select="sex" /></td>
			<td hight="15" align="center"><xsl:value-of select="birthday" /><br /></td>
		</tr>
		</xsl:for-each>
		</table>
		</center>
	</body>
</html>
</xsl:template>
</xsl:stylesheet>