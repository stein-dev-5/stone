const API = 'http://localhost:8080/api';
let state = { s: 0, t: 0, c: 0, m: 0, o: false, w: 2 };
let busy = false;

async function start(mode) {
    const init = parseInt(document.getElementById('init').value) || 10;
    const tgt = parseInt(document.getElementById('tgt').value) || 50;
    
    try {
        const res = await fetch(`${API}/new?mode=${mode}&initial=${init}&target=${tgt}`);
        state = await res.json();
        document.getElementById('setup').style.display = 'none';
        document.getElementById('game').style.display = 'block';
        update();
        if (state.m === 1 && state.c === 1 && !state.o) {
            setTimeout(compMove, 500);
        }
    } catch (e) {
        alert('Ошибка');
    }
}

async function move(act, val) {
    if (state.o || busy || (state.m === 1 && state.c === 1)) return;
    
    busy = true;
    const btns = document.querySelectorAll('.actions button');
    btns.forEach(b => b.disabled = true);
    
    try {
        const res = await fetch(`${API}/move?action=${act}&value=${val}`);
        state = await res.json();
        update();
        
        if (state.o) {
            showOver();
            busy = false;
            return;
        }
        
        if (state.m === 1 && state.c === 1 && !state.o) {
            setTimeout(compMove, 800);
        } else {
            busy = false;
            btns.forEach(b => b.disabled = false);
        }
    } catch (e) {
        alert('Ошибка');
        busy = false;
        btns.forEach(b => b.disabled = false);
    }
}

async function compMove() {
    if (state.o || busy) return;
    busy = true;
    
    try {
        const res = await fetch(`${API}/move?action=0&value=0`);
        state = await res.json();
        update();
        if (state.o) showOver();
    } catch (e) {
        alert('Ошибка');
    } finally {
        busy = false;
    }
}

function update() {
    document.getElementById('stones').textContent = state.s;
    document.getElementById('target').textContent = state.t;
    document.getElementById('player').textContent = 
        state.c === 0 ? (state.m === 0 ? 'Игрок 1' : 'Ваш ход') :
        (state.m === 0 ? 'Игрок 2' : 'Ход компьютера...');
    
    const d = document.getElementById('display');
    d.innerHTML = '';
    
    if (state.s <= 50) {
        const frag = document.createDocumentFragment();
        for (let i = 0; i < state.s; i++) {
            const s = document.createElement('div');
            s.className = 'stone';
            frag.appendChild(s);
        }
        d.appendChild(frag);
    } else {
        const c = document.createElement('div');
        c.className = 'count';
        c.textContent = state.s;
        d.appendChild(c);
    }
    
    const btns = document.querySelectorAll('.actions button');
    btns.forEach(b => {
        b.disabled = state.o || busy || (state.m === 1 && state.c === 1);
    });
}

function showOver() {
    const w = document.getElementById('winner');
    if (state.w === 0) {
        w.textContent = state.m === 0 ? 'Победил Игрок 1!' : 'Вы победили!';
    } else {
        w.textContent = state.m === 0 ? 'Победил Игрок 2!' : 'Победил компьютер!';
    }
    document.getElementById('over').style.display = 'flex';
}

function menu() {
    document.getElementById('setup').style.display = 'block';
    document.getElementById('game').style.display = 'none';
    document.getElementById('over').style.display = 'none';
    busy = false;
}

