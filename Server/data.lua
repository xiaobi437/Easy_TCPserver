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
<!-- ���������� -->
<div class="loading-overlay">
    <div class="loading-content">
        <div class="loading-spinner"></div>
        <p style="font-size: 15px;color: #ffffff;margin-top: 10px;">������...</p>
        <button class="cancel-button" onclick="closeOverlay(event)" id='cancel-button'>ȡ��</button>
    </div>
</div>
<script>

    document.addEventListener('DOMContentLoaded', function() {
        // ��������show-loading���Ԫ�ص���¼�
        var show_loading = document.querySelectorAll('.show-loading');
        if(show_loading){
            show_loading.forEach(function(element) {
                element.addEventListener('click', function() {
                    showloadingOverlay();
                });
            });
        }
        // ������a���ӵ���¼�
        var links = document.querySelectorAll('a');
        if(links){
            links.forEach(function(link) {
                var onclickAttr = link.getAttribute('onclick');
                // ���Դ� onclick �ģ��������� onclick="atarget(this)"
                if (!onclickAttr || onclickAttr === 'atarget(this)') {
                // if (!link.onclick) { //���Դ�onclick��
                    link.addEventListener('click', function(event) {
                        var href = link.getAttribute('href').toLowerCase();
                        if (href && !href.startsWith('javascript:') && !href.startsWith('#')) {
                            showloadingOverlay();
                        }
                    }, true); // ��� true ����ʹ���ڲ���׶δ���
                }
            });
        }
        //��������ť(pc��laowang)
        document.body.addEventListener('submit', function(event) {
            if (event.target && event.target.id === 'dtpaytipform') {
                showloadingOverlay(1);
            }
        }, true);
    });
    //���ڱ䶯ʱ����
    document.addEventListener("visibilitychange", function() {
        hideloadingOverlay();
    });

    // ��ʾ���ֲ�
    function showloadingOverlay(hidden_cancel = 0) {
        if(hidden_cancel){
            document.getElementById('cancel-button').style.display = 'none';
        }
        document.querySelector('.loading-overlay').style.display = 'block';
    }

    // �������ֲ�
    function hideloadingOverlay() {
        document.querySelector('.loading-overlay').style.display = 'none';
    }

    // ���ȡ����ť���ر����ֲ���ֹĬ���¼�������������ת��
    function closeOverlay(event) {
        window.stop();
        event.preventDefault(); // ��ֹ������ת
        hideloadingOverlay();
        return false;
    }