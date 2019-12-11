drop table if exists post_terms;
create table if not exists post_terms (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    post_id INTEGER NOT NULL DEFAULT 0,
    term_id INTEGER NOT NULL DEFAULT 0
);
drop table if exists terms;
create table if not exists terms (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    slug TEXT NOT NULL
);
drop table if exists posts;
create table if not exists posts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    slug TEXT NOT NULL,
    excerpt TEXT NOT NULL,
    content TEXT NOT NULL,
    pubdate TEXT NOT NULL,
    tags TEXT NOT NULL
);
