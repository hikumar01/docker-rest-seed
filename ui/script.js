document.addEventListener('load', function() {
    const usernameBtn = document.getElementById('usernameBtn');

    const observer = new MutationObserver(function(mutationsList) {
        for (let mutation of mutationsList) {
            if (mutation.type === 'childList') {
                updateButtonVisibility();
            }
        }
    });
    observer.observe(usernameBtn, { childList: true });

    updateButtonVisibility();

    setInterval(() => {
        usernameBtn.innerText = 'Time: ' +  new Date().toLocaleTimeString();
    }, 1000);
});

function updateButtonVisibility() {
    const usernameBtn = document.getElementById('usernameBtn');
    usernameBtn.style.display = usernameBtn.textContent.trim().toLowerCase() === ''? 'none' : 'inline';
}

function handleSubmit(event) {
    event.preventDefault();
    const name = document.getElementById('name').value;
    document.getElementById('usernameBtn').innerText = name;
}

function usernameBtnClicked(event) {
    document.getElementById('usernameBtn').innerText = '';
}

function fetchData(event) {
    let headersText = '';
    document.getElementById('responseData').value = '';
    fetch('/api/hello')
        .then(response => {
            response.headers.forEach((value, key) => {
                headersText += `${key}: ${value}` + '\n';
            });
            return response.json();
        })
        .then(data => {
            headersText += JSON.stringify(data, null, 2);
        })
        .catch(error => {
            headersText += '\nError: ' + error.message;
        })
        .finally(() => {
            console.log(headersText);
            document.getElementById('responseData').value = headersText;
        });
}
