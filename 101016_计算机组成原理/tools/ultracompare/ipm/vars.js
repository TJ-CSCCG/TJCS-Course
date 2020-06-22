var langStr = 'ch';
var ucNewPrice = "$49.95";
var ucUpgradePrice = "$24.95";
var ueUcBundleNewPrice = "$89.95";      //new bundle cost
var ueUcBundleRetailPrice = "$109.90";   //bundle retail price
var ueUCBundleYouSavePercentage = "40% on UC";
var ueUcUpgradeBundlePrice = "$44.95";    // bundle upgrade price
var ueUpgradeToUeUcBundlePrice = "$59.95";
var strReminderText = '剩余日子: ';

var strTopRightBlockContent = 
    '<p class="register_title">Register</p>' +
    '<p><img src="images/ucbox_sm.gif" alt="UltraCompare" border="0" class="ucbox" />注册: <br /><span class="price" id="ucNewPrice">$##.##</span></p>' +
    '<a href="#" onClick="purchaseLink(\'BuyUC\', langStr); return false;" target="_blank"><p class="buy_button">购买</p></a>' +
    '<p class="upgrade_text">升级: <span class="price" id="ucUpgradePrice">$##.##</span></p>' +
    '<a href="#" onClick="purchaseLink(\'UpgradeUC\', langStr); return false;" target="_blank"><p class="upgrade_button">升级</p></a><br />';
                  
var strBottomRightBlockContent = 
    '<p class="register_title">UE/UC </p>' +
    '<p><img src="images/ue_uc_box.gif" alt="UltraEdit UltraCompare 捆绑" border="0" class="bundlebox" />两者注册: <span class="price" id="ueUcBundleNewPrice">$##.##</span></p>' +
    '<a href="#" onClick="purchaseLink(\'BuyUEUCBundle\', langStr); return false;" target="_blank"><p class="buy_button">购买</p></a>'+
    '<p class="save_on_uc_text">Save 40% on UC!</p> ';

var strHeaderSubText= 
    '<h2>这个是没有注册的软件。 </h2>' +
    '<p>您是利用试用版本来使用这个软件。 </p> ';
    
var strUpgradeReminderHeaderSubText = 
    '<h2>您的免费升级期已过</h2> ' +
    '<p>今天升级，可获得了下一年的免费升级，同时节省50 ％</p>';

var strStdTopHeadline = 'UltraCompare 试用版本提示';
var strHurryTopHeadline = '你的试用版本几乎到期';
var strExpiredTopHeadline = '哦... 你的试用版本已经到期';
var strUpgradeReminderTopHeadline = '授权更新提醒';

                          
// ----- DIALOG CONTENT ---------//     
var defaultContent =
'<h2>谢谢试用 UltraCompare</h2>'+ 
'<p class="subtext">您是利用试用版本来使用这个软件。</p>'+ 
'<p>你的 UltraCompare 是没有注册。如果你想继续使用这个程式，请购买授权。 </p>'+ 
'<p><a href="http://www.ultraedit.com/redirects/registration/en/uc_register.html?utm_source=UltraCompare&utm_medium=ipm&utm_campaign=default" target="_blank">按这里注册</a></p>'+ 
'<p style="clear: all"><b>接受 Visa, MasterCard, Amex</b></p>'+ 
'<p>哲想方案（北京）科技有限公司(IDM 中国)<br />北京市海淀区西三环北路50号院<br />H豪柏大厦B1座606室, 邮编 ##.##8<br />傳真: ##.##-##.##628<br />电邮: <a href="mailto:sales@ultraedit.cn" target="_blank">sales@ultraedit.cn</a></p>'+ 
'<p><a href="http://www.ultraedit.com/company/contact_us.html?utm_source=ultracompare&utm_medium=ipm&utm_campaign=default" target="_blank">如果有问题，请跟我们联络</p>';
                     
var welcomeContent =
		'<h2>谢谢您评估 UltraCompare</h2>'+
		'<p>利用 IDM 用户社区资源中心最大化您的生产力 - 包括支持、下载、新闻等...</p>'+
		'<div class="powertips">'+
		'<p><span class="resource_title">力量提示/教程</span><br />'+
		'"How-tos" 是给最新版本的功能，和现有功能的深入教程。</p>'+
		'</div>'+
		'<div class="tech_support">'+
		'<p><span class="resource_title">技术支持</span><br />'+
		'有问题?  有答案!  利用您的技术资源令 IDM 软件为您服务。</p>'+
		'</div>'+
		'<div class="forums">'+
		'<p><span class="resource_title">用户对用户论坛</span><br />'+
		'参加论坛跟 IDM 产品的爱好者分享 - 分享技巧，得到意见等。</p>'+
		'</div>'+
		'<p class="resource_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/resources.html\'); return false;" target="_blank">访问资源中心</a></p>';
    
var textModeContent = 
    '<img src="images/text_compare_icon.jpg" alt="文本比较" border="0" class="tip_screenshot" />'+
    '<h2>文本比较</h2>'+
    '<p class="subtext">比较文本文件、源代码、 Word 文件\'s 和更多...</p>'+
    '<p>UltraCompare 支持2方和3方比较 - 对于版本控制、验证备份和追踪你的队伍的更动。</p>' +
    '<p style="clear: both">有联系的行形象化地把文件的分别连起来，所以你可以容易地识别和合并文件的分别。' +
    '<p>UltraCompare 突出在文本的每一个字符的分别，令形象化的文件分别检查更快。</p>';
    
    
var folderModeContent = 
    '<img src="images/folder_compare_icon.jpg" alt="目录比较" border="0" class="tip_screenshot" />'+
    '<h2>目录比较</h2>'+
    '<p class="subtext">比较你的目录、 .zip 档案和更多有关目录方式</p>' +
    '<p>利用目录方式很快地比较本地、网络或远端目录 (利用内置的 FTP/SFTP 支持)。 </p>'+
    '<p style="clear: both">深的目录结构?  不是问题... 目录方式是给大目录结构的最好化性能，让你开始运作高级的目录，同时 UltraCompare 继续在背景进行子目录。</p>'+
    '<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/recursive_compare.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var mergeContent = 
    '<img src="images/merge_icon.jpg" alt="合并" border="0" class="tip_screenshot" />'+
    '<h2>合并分别</h2>'+
    '<p class="subtext">利用块和行方式合并合并分别</p>'+
    '<p>UltraCompare 的强大的合并选择项给你在比较文件分别上完全控制。</p>'+
    '<p>UltraCompare 专业版强大和直觉的功能节省你很多时间。你可以利用合并工具栏、在比较窗格中的比较像标或比较菜单选择项合并分别。 </p>'+
    '<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/block_line_mode_merge.html\'); return false;" target="_blank">按这里了解更多</a></p>';
    
var defaultContent = 
    '<h2>谢谢试用 UltraCompare</h2>'+ 
    '<p class="subtext">您是利用试用版本来使用这个软件。</p>'+ 
    '<p>你的 UltraCompare 是没有注册。如果你想继续使用这个程式，请购买授权。 </p>'+ 
    '<p><a href="#" onClick="purchaseLink(\'BuyUC\', langStr); return false;" target="_blank">按这里注册 UltraCompare</a></p>'+ 
    '<p style="clear: all"><b>接受 Visa, MasterCard, Amex</b></p>'+ 
    '<p>IDM Computer Solutions, Inc.<br />5559 Eureka Dr. Ste. B<br />Hamilton, OH ##.##<br />Fax: (513) 8##.##00<br />Email: <a href="mailto:idm@idmcomp.com" target="_blank">idm@idmcomp.com</a></p>'+ 
    '<p><a href="#" onClick="openLink(\'http://www.ultraedit.com/company/contact_us.html\'); return false;" target="_blank">如果有问题，请跟我们联络</a> ';
    
var benefitsContent =
    '<h2>注了册的好处</h2>'+ 
    '<div class="benefits">'+ 
    '<p><span class="subtext">为您的问题解答 - 终身支持</span><br />'+ 
    '世界级的技术支持 - 标准反应时间: 30 分钟或更少。</p>'+ 
    '<p><span class="subtext">一年免费升级</span><br />'+ 
    '注册容许您一年的升级，包括重要和次要的版本。</p>'+ 
    '<p><span class="subtext">社区好处</span><br />'+ 
    '超过二百万用户、丰富网上资源、力量提示、用户主导开发模式、月刊和论坛等 </p>'+ 
//    '<div class="bonus">'+       
//      '<p><span class="subtext">用户基础的授权!</span><br />'+ 
//      '拥有膝上型轻便电脑或额外 PC? 注册可以容许你安装在多个电脑上 - 条件是你是唯一用软件的用户。</p>'+ 
//    '</div>'+ 
    '</div> <!-- end benefits -->';
    
var editTextContent =
    '<img src="images/edit_text_icon.jpg" alt="Edit Text" border="0" class="tip_screenshot" />'+ 
    '<h2>编辑文本</h2>'+ 
    '<p class="subtext">在比较窗格中直接编辑文本</p>'+ 
    '<p style="clear: both">除了比较分别，有时候你需要做更多... 你需要直接编辑文件!  编辑文件对于提高你的工作效率是很重要的。</p>'+ 
    '<p>文本比较给你两个方法编辑文件: 在比较窗格直接编辑或在应用程序的下方的活动行窗进行编辑。在窗格中直接编辑，点击窗格和键入... 这是很容易!</p>'+ 
    '<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/editing_files.html\'); return false;" target="_blank">按这里了解更多</a></p>';    

var sessionsContent =
    '<img src="images/sessions_icon.gif" alt="对话" border="0" class="tip_screenshot" />'+ 
    '<h2>Sessions</h2>'+ 
    '<p class="subtext">用对话管理多个比较组合</p>'+ 
    '<p>开多个应用程序对你工作有影响，所以 UltraCompare 给你对话，让你简化比较!</p>'+ 
    '<p>对话让你管理多个比较组合 - 不论何方式 - 全在一个应用程序。 标签界面让你容易地切换多个对话。</p>'+ 
    '<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/ultracompare_sessions.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var webCompareContent = 
    '<img src="images/web_compare_icon.gif" alt="网比较" border="0" class="tip_screenshot" />'+ 
    '<h2>Compare Web Files</h2>'+ 
    '<p class="subtext">比较本地和远端文件</p>'+ 
    '<p>如果你处理网文件，你可能已经习惯了通过 FTP 下载文件或观看来源，保存文本，然后比较。</p>'+
    '<p style="padding-top: 3px">你是同意 - 一定有更容易的方法!</p> '+
    '<p>利用网比较，在文件途径栏输入 URL，然而去...  容易! 网比较除掉额外的步骤和引入网文件的内容作一个快的比较。</p> '+
    '<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/web_compare.html\'); return false;" target="_blank">按这里了解更多</a></p>';
    
    
    
var wordDocCompare = 
'<img src="images/word_compare.gif" alt="Word 比较" border="0" class="tip_screenshot" />'+ 
'<h2>Word Doc/RTF 比较</h2>'+ 
'<p class="subtext">比较 Word Docs 和 RTF 文件</p>'+ 
'<p>你有一个 Word 文件跟其他人协作吗? 容易地跟踪所有的改变。</p>'+ 
'<p style="clear: both">UltraCompare 专业版支持 Microsoft 和 RTF 文件的比较和合并。如果你有知道你作的改变，UltraCompare\ 的 Word Doc 比较是你想的!</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/word_doc_compare.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var folderModeFiltersContent = 
'<img src="images/folder_mode_filters_icon.gif" alt="目录方式过滤器" border="0" class="tip_screenshot" />'+ 
'<h2>过滤比较结果</h2>'+ 
'<p class="subtext">简单化比较结果</p>'+ 
'<p>比较目录结构是一个困难的工作，特别是你的目录包含很多文件/目录。 </p>' +
'<p style="clear: both">如果你想利用结果组工作更出色，你需要忽略特定的类型/目录或只是显示特定的结果，例如 "不同文件". </p>'+ 
'<p>UltraCompare 提供多个方法过滤你的结果，给你焦点比较组。</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/filtering_files.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var compareProfilesContent = 
'<img src="images/profiles_icon.gif" alt="外观" border="0" class="tip_screenshot" />'+ 
'<h2>比较外观</h2>'+ 
'<p class="subtext">需要利用特定设置比较同一个文件和目录?</p>'+ 
'<p>用比较外观保存和再用你的比较设置。</p>'+ 
'<p style="clear: all">比较外观让你保存比较设置和在现在或将来对话再用他们。 '+
'<p>比较外观让你以名称、保存选择项、记忆文件和目录路径等保存设置。你可以引入、输出和分享你的外观!</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/custom_user_profiles.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var ignoreOptionsContent = 
'<img src="images/ignore_options_icon.gif" alt="外观" border="0" class="tip_screenshot" />'+ 
'<h2>文件忽略选择项</h2>'+ 
'<p class="subtext">用可调整的忽略选择项忽略不相关的文本</p>'+ 
'<p>很多程序员需要比较源代码而不是注解... 忽略选择项让你做这个，而且做的更多!</p>'+ 
'<p>你是否有两个几乎一样的文件，除了 tabs、spaces 或行终止者? 忽略选择项让你比较文件的内容和忽略 whitespace。略选择项可以用在不同分别管理的途径上。 </p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/ignore_options.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var shellIntegrationContent =
'<img src="images/shell_integration_icon.gif" alt="Shell 整合" border="0" class="tip_screenshot" />'+ 
'<h2>Shell 整合</h2>'+ 
'<p class="subtext">在视窗浏览器的右击菜单迅速的执行文件和目录比较行动</p>'+ 
'<p style="clear: both">如果你是经常比较文件和目录，你需要一个快、有效率和容易使用。UltraCompare 利用 Shell 整合视窗浏览器来令比较行动更顺。</p>'+ 
'<p>Shell 整合让你在视窗浏览器标记文件和执行比较 - 全部从方便的右击菜单。</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/ultracompare_shell_integration.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var ftpCompareContent = 
'<img src="images/compare_ftp_folders.gif" alt="FTP Compare" border="0" class="tip_screenshot" />'+ 
'<h2>FTP/SFTP 支援 - 远端文件和目录比较</h2>'+ 
'<p class="subtext">用整合 FTP/SFTP 支援比较和同步远端文件和目录</p>'+ 
'<p>UltraCompare\ 的 FTP/SFTP 目录比较让你访问在服务器的远端文件和目录。执行目录比较和同不你的远端文件和目录。比较、合并和保存... 很容易! </p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/compare_FTP_directories.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var favoriteFilesAndFoldersContent = 
'<img src="images/favorite_files_folders_icon.gif" alt="Favorites" border="0" class="tip_screenshot" />'+ 
'<h2>喜爱文件目录</h2>'+ 
'<p class="subtext">利用喜爱来保存时常用的文件和目录</p>'+ 
'<p>你是否经常比较同一组文件和目录? 如果是，你需要喜爱!</p>'+ 
'<p style="clear: both">喜爱是一个很方便的途径来书签你经常用的文件和目录，比较的时候就会更快。</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/bookmark_favorite_files-folders_in_ultracompare.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var snippetCompareContent =
'<img src="images/compare_text_snippets_icon.gif" alt="Favorites" border="0" class="tip_screenshot" />'+ 
'<h2>比较文本摘录</h2>'+ 
'<p class="subtext">不需要开/保存文件，而比较文本摘录</p> '+
'<p>只是拷贝/粘贴你的文本到比较框! </p>'+ 
'<p style="clear: both">你可以从很多途径拷贝/粘贴文本，例如电邮、Word Documents、网页等。你会喜爱文本摘录比，他会提高你的工作效率。</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/compare_code_snippets.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var archiveCompareContent =
'<img src="images/zip_archive_icon.gif" alt="Archive Compare" border="0" class="tip_screenshot" />'+ 
'<h2>存档比较</h2>'+ 
'<p class="subtext">用存档比较比较 .zip、.jar和 .rar 存档</p>'+ 
'<p>有存档?  UltraCompare 的存档比较让你比较.zip、.rar 和 Java .jar 存档的内容。</p>'+ 
'<p>存档比较不单是支持以上的受欢迎的格外，它也是支持密码保护的 .zip 存档!</p>'+ 
'<p>利用存档比较和检查在文件系统上的存档和目录。存档比较很容易-to-use!</p>';

var browserViewContent =
'<img src="images/web_compare_icon.gif" alt="网比较" border="0" class="tip_screenshot" />'+ 
'<h2>HTML 浏览器视图</h2>'+ 
'<p class="subtext">利用HTML 预览器形象化地检查你的源代码</p>'+ 
'<p>如果你比较/合并 HTML 文件和你需要看它在浏览器变化，你是找到了对的地方!'+ 
'<p style="clear: both; padding-top: 5px">UltraCompare 支持在比较框的综合浏览器，它让你预览 生产的 HTML。比较、合并和/或编辑，然后点击浏览器视图进行快的形象化的变化检查。</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/visually_inspect_HTML.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var customColorsContent =
'<img src="images/custom_colors_icon.gif" alt="自定颜色" border="0" class="tip_screenshot" />'+ 
'<h2>自定颜色</h2>'+ 
'<p class="subtext">自定界面的颜色</p>'+ 
'<p>是否需要自定颜色和主题 UltraCompare 用法? </p>'+ 
'<p style="clear: both">UltraCompare 让你容易自定颜色方案。有一个喜欢的颜色方案? 习惯了你自己的颜色设置? 设置你的颜色方案! </p>'+ 
'<p>还有，你可以创造和保存多个颜色方案和转换容易。</p> '+
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/customize_colors.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var quickDifferenceCheckContent =
'<img src="images/command_line_quick_diff.gif" alt="自定颜色" border="0" class="tip_screenshot" />'+ 
'<h2>命令行分别快检查</h2>'+ 
'<p class="subtext">执行命令行分别快检查，不用打开 UltraCompare 比较你的文件 </p>'+ 
'<p style="clear: both">命令行分别快检查你的文件的分别，不用显示 GUI - 令比较特别快!</p>'+ 
'<p>如果你需要检查你的文件是否不同，不用知道他们的分别，你可以用一个命令行!</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/quick_diff_check.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var manualAlignContent =
'<img src="images/manual_allign_icon.gif" alt="手动排列" border="0" class="tip_screenshot" />'+ 
'<h2>手动排列</h2>'+ 
'<p class="subtext">在来源文件手动配合/同步行</p>'+ 
'<p>有时后你的文件包含类似的资料，UC 没有方法知道配对的行。所以你需要有一个方法手动同步比较，显示文件“like”的部分。手动同步令 UC 更聪明，因为它知道要同步的行。只是右击任何两行和同步从这个点继续比较。</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/manual_sync.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var outputContent = 
'<img src="images/report_icon.gif" alt="Output" border="0" class="tip_screenshot" />'+ 
'<h2>Reporting</h2>'+ 
'<p class="subtext">输出和保存多个格式的结果</p>'+ 
'<p>输出和保存多个格式的结果: 旁对旁、 context 内等...</p>'+ 
'<p style="clear: both">如果你想很快看到比较文件的不同之处，只是利用 Difference Summary 报告。  如果你需要详细的报告，你可以产生一个结果文件。你可以在选择项的对话框设置输出方式。</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/export_save_text_compare_output.html\'); return false;" target="_blank">按这里了解更多</a></p>';

var configureTimeDateContent = 
'<img src="images/custom_date_format_icon.gif" alt="自定日期格式" border="0" class="tip_screenshot" />'+ 
'<h2>时间和日期显示</h2>'+ 
'<p class="subtext">自定时间和日期显示格式</p>'+ 
'<p>当执行目录比较，你可能不太介意一个半个小时的细节，当是你可能想看一个月结果。</p>'+ 
'<p>你不应每一次作比较的时时候想时间和日期的格式，它是应跟你的意见。UltraCompare 让你自定时间和日期的格式。放置后忘记它... 容易!</p>'+ 
'<p class="powertip_link"><a href="#" onClick="openLink(\'http://www.ultraedit.com/support/tutorials_power_tips/ultracompare/customize_time_date.html\'); return false;" target="_blank">按这里了解更多</a></p>';



// ----- END DIALOG CONTENT ---------//

// ---- START RIGHT/LEFT CONTENT ----//

var rightSideContent =
    '<img src="images/ue_uc_box_big.gif" alt="UltraEdit UltraCompare Bundle Box" border="0" class="ue_uc_box_big" />'+ 
		'<h2>UE/UC Bundle</h2>'+ 
		'<p class="ue_uc_bundle_offer">零售价: <span class="retail_price" id="ueUcBundleRetailPrice">$##.##</span><br />'+ 
			'您的价格: <span class="price" id="ueUcBundleNewPrice">$##.##</span><br />'+ 
			'您的折扣: <span class="you_save" id="ueUCBundleYouSavePercentage">##</span>'+ 
		'</p>'+ 
    '<p style="clear: both">UltraEdit/UltraCompare 是一个完全综合的完整文本编辑方案。 购买 UE/UC 方案，省 UC 的 40%!</p>'+     
	  '<div class="register_upgrade_container">'+ 
	  '<div class="best_value">最好价格</div>' +
	  	'<div class="register">'+ 
				'<p class="register_text">两者注册 <span class="price" id="ueUcBundleNewPrice">$##.##</span></p>'+ 
				'<a href="#" onClick="purchaseLink(\'BuyUEUCBundle\', langStr); return false;" target="_blank"><p class="buy_button">购买</p></a>'+ 
			'</div>'+ 
			'<div class="upgrade">'+ 
				'<p class="register_text">UE 升级 + UC <span class="price" id="ueUpgradeToUeUcBundlePrice">$##.##</span></p>'+ 
				'<a href="#" onClick="purchaseLink(\'UpgradeUE2UEUCBundle\', langStr); return false;" target="_blank"><p class="buy_button">升级</p></a>'+ 
		'</div>';

var leftSideContent =
    '<img src="images/ucbox_big.gif" alt="UltraEdit 盒装" border="0" class="ucbox_big" />'+ 
    '<h2>UltraCompare Pro</h2>'+ 
    '<p>UltraCompare Pro 是一个强大的比较/合并应用程序，它可以让你追踪文件、目录和 .zip 档案的分别! '+ 
    '<div class="bonus">'+ 
			  '<p><span class="subtext">一年免费升级</span><br />'+ 
    		'注册容许您一年的升级，包括重要和次要的版本。</p>'+ 
    '</div>'+ 
    '<div class="register_upgrade_container">'+ 
    '<div class="register">'+ 
    '<p class="register_text">注册 <span class="price" id="ucNewPrice">$##.##</span></p>'+ 
    '<a href="#" onClick="purchaseLink(\'BuyUC\', langStr); return false;" target="_blank"><p class="buy_button">购买</p></a>'+ 
    '</div>'+ 
    '<div class="upgrade">'+ 
    '<p class="register_text">UC 升级<span class="price" id="ucUpgradePrice">$##.##</span></p>'+ 
    '<a href="#" onClick="purchaseLink(\'UpgradeUC\', langStr); return false;" target="_blank"><p class="buy_button">升级</p></a>'+ 
    '</div>';


// ---- END RIGHT/LEFT CONTENT ----//



// ---- START RIGHT/LEFT CONTENT FOR UPGRADE REMINDER MESSAGE----//

var upgradeRightSideContent =
    '<img src="images/ue_uc_box_big.gif" alt="UltraEdit UltraCompare Bundle Box" border="0" class="ue_uc_box_big" />'+ 
		'<h2>UE/UC Bundle</h2>'+ 
		'<p class="ue_uc_bundle_offer">Upgrade both today for<br /> Only: <span class="price" id="ueUcUpgradeBundlePrice">$##.##</span></p>'+ 
		'<p style="clear: both">Using UltraEdit? Save an additional 20% when you upgrade UltraCompare and UltraEdit together, plus receive another year of free upgrades for both products.</p>'+ 
	  '<div class="register_upgrade_container">'+ 
			'<div class="register" style="height: 70px; background-image: url(images/best_value_arrow.gif); background-position: 100% 50%; background-repeat: no-repeat;">'+ 
				'<p style="padding-top: 23px; padding-left: 23px; font-weight: bold">BEST VALUE</p>'+ 
			'</div>'+ 
			'<div class="upgrade">'+ 
				'<p class="register_text">UE/UC Upgrade<span class="price" id="ueUcUpgradeBundlePrice">$##.##</span></p>'+ 
				'<a href="#" onClick="purchaseLink(\'UpgradeUEUCBundle\', langStr); return false;" target="_blank"><p class="buy_button">UPGRADE</p></a>'+ 
		'</div>';

var upgradeLeftSideContent =
    '<img src="images/ucbox_big.gif" alt="UltraCompare Box" border="0" class="ucbox_big" />'+ 
    '<h2>UltraCompare</h2>'+ 
    '<p>想了解UltraCompare最新的功能？  <a href="#" onClick="openLink(\'http://www.ultraedit.com/products/ultracompare/new_feature_tour.html\'); return false;" target="_blank">See </a> what\'s new and improved since your last upgrade.</p>'+ 
    '<div class="bonus_upgrade">'+ 
      '<p><span class="subtext">升级再赠... 1年的升级！</span><br />'+ 
      '升级到最新版本，外加一年的 <b>免费的</b> 升级包括所有主要/次要的发布</p>'+ 
    '</div>'+ 
    '<div class="register_upgrade_container">'+ 
    '<div class="register">'+ 
	    '<p style="text-align: left; padding-top: 4px; padding-left: 5px"><strong>升级定价</strong><br>'+ 
	    '注册用户收到50 ％免费的 <br>零售价！</p>'+ 
    '</div>'+ 
    '<div class="upgrade">'+ 
    '<p class="register_text">UC 升级<span class="price" id="ucUpgradePrice">$##.##</span></p>'+ 
    '<a href="#" onClick="purchaseLink(\'UpgradeUC\', langStr); return false;" target="_blank"><p class="buy_button">升级</p></a>'+ 
    '</div>';


// ---- END RIGHT/LEFT CONTENT FOR UPGRADE REMINDER MESSAGE----//

