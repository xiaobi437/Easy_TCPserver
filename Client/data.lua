<style>.loading-overlay{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.5);z-index:1000;}.loading-content{position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);text-align:center;}.cancel-button{display:inline-block;margin-top:20px;padding: 5px 15px;background-color:#da8080;color:white;border:none;border-radius:15px;cursor:pointer;font-size:12px;}
.loading-spinner {
  height: 30px;
  aspect-ratio: 2.5;
  --_g: no-repeat radial-gradient(farthest-side, #fff 90%, #0000);
  background:var(--_g), var(--_g), var(--_g), var(--_g);
  background-size: 20% 50%;
  animation: l43 1s infinite linear; 
}@keyframes l43 {
  0%     {background-position: calc(0*100%/3) 50% ,calc(1*100%/3) 50% ,calc(2*100%/3) 50% ,calc(3*100%/3) 50% }
  16.67% {background-position: calc(0*100%/3) 0   ,calc(1*100%/3) 50% ,calc(2*100%/3) 50% ,calc(3*100%/3) 50% }
  33.33% {background-position: calc(0*100%/3) 100%,calc(1*100%/3) 0   ,calc(2*100%/3) 50% ,calc(3*100%/3) 50% }
  50%    {background-position: calc(0*100%/3) 50% ,calc(1*100%/3) 100%,calc(2*100%/3) 0   ,calc(3*100%/3) 50% }
  66.67% {background-position: calc(0*100%/3) 50% ,calc(1*100%/3) 50% ,calc(2*100%/3) 100%,calc(3*100%/3) 0   }
  83.33% {background-position: calc(0*100%/3) 50% ,calc(1*100%/3) 50% ,calc(2*100%/3) 50% ,calc(3*100%/3) 100%}
  100%   {background-position: calc(0*100%/3) 50% ,calc(1*100%/3) 50% ,calc(2*100%/3) 50% ,calc(3*100%/3) 50% }
}</style>
<!-- 加载中遮罩 -->
<div class="loading-overlay">
    <div class="loading-content">
        <div class="loading-spinner"></div>
        <p style="font-size: 15px;color: #ffffff;margin-top: 10px;">加载中...</p>
        <button class="cancel-button" onclick="closeOverlay(event)" id='cancel-button'>取消</button>
    </div>
</div>
<script>

    document.addEventListener('DOMContentLoaded', function() {
        // 监听带有show-loading类的元素点击事件
        var show_loading = document.querySelectorAll('.show-loading');
        if(show_loading){
            show_loading.forEach(function(element) {
                element.addEventListener('click', function() {
                    showloadingOverlay();
                });
            });
        }
        // 监听带a链接点击事件
        var links = document.querySelectorAll('a');
        if(links){
            links.forEach(function(link) {
                var onclickAttr = link.getAttribute('onclick');
                // 忽略带 onclick 的，但不忽略 onclick="atarget(this)"
                if (!onclickAttr || onclickAttr === 'atarget(this)') {
                // if (!link.onclick) { //忽略带onclick的
                    link.addEventListener('click', function(event) {
                        var href = link.getAttribute('href').toLowerCase();
                        if (href && !href.startsWith('javascript:') && !href.startsWith('#')) {
                            showloadingOverlay();
                        }
                    }, true); // 添加 true 参数使其在捕获阶段触发
                }
            });
        }
        //监听购买按钮(pc，laowang)
        document.body.addEventListener('submit', function(event) {
            if (event.target && event.target.id === 'dtpaytipform') {
                showloadingOverlay(1);
            }
        }, true);
    });
    //窗口变动时隐藏
    document.addEventListener("visibilitychange", function() {
        hideloadingOverlay();
    });

    // 显示遮罩层
    function showloadingOverlay(hidden_cancel = 0) {
        if(hidden_cancel){
            document.getElementById('cancel-button').style.display = 'none';
        }
        document.querySelector('.loading-overlay').style.display = 'block';
    }

    // 隐藏遮罩层
    function hideloadingOverlay() {
        document.querySelector('.loading-overlay').style.display = 'none';
    }

    // 点击取消按钮，关闭遮罩并阻止默认事件（例如链接跳转）
    function closeOverlay(event) {
        window.stop();
        event.preventDefault(); // 阻止链接跳转
        hideloadingOverlay();
        return false;
    }