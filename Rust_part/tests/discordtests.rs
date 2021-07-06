mod handlers;
mod botrunner;
mod consts;

#[cfg(test)]
mod tests {
    use vecbot::secrettoken::{getchannelid, gettoken, getwatchertoken};
    use vecbot::bot::{
        create_vec_bot, DefaultHandler
    };
    use vecbot::constants::CRASH;
    use std::result::Result;
    use std::io::Error;
    use tokio;
    use serenity;
    use serenity::{
        http::Http,
        model::{
            id::{ChannelId},
        },
        utils::MessageBuilder,
        client::{EventHandler}
    };
    use std::{thread, time::{Duration}};
    
    use std::sync::{Mutex, Arc};
    
    use super::handlers::{
        RECEIVE_CONTENT,
        RECEIVE_EMBED_CONTENT,
        RECEIVE_IMAGE_EMBED_CONTENT,
        ReceiveEmbedMessageHandler, 
        ReceiveMessageHandler, 
        ReceiveImageEmbedMessageHandler,
        StartOtherBotHandler,
        DoNothing
    };
    use super::consts::TEST_IMAGE;
    use super::botrunner::{
        start_running_bot,
        RunningBot
    };
    
    #[test]
    fn token_obtainable() -> Result<(), Error> {
        println!("fetching token...");
        let _token = gettoken();
        Ok(())
    }

    #[tokio::test]
    async fn bot_creatable() -> Result<(), Error> {
        let token = gettoken();
        let tokenstr = token.as_str();
        let _outcome = create_vec_bot(tokenstr);
        Ok(())
    }

    #[tokio::test]
    async fn bot_runnable() -> Result<(), Error> {
        let token = gettoken();
        let tokenstr = token.as_str();
        let mut client = create_vec_bot(tokenstr).await;
        let shard_man = client.shard_manager.clone();

        let _ = client.start();
        println!("bot shutting down...");
        shard_man.lock().await.shutdown_all().await;

        Ok(())
    }


    #[tokio::test]
    async fn can_send_and_receive_a_message() -> Result<(), Error> {
        println!("starting two bots...");
        let token1 = gettoken();
        
        println!("check whether messages can be sent between them...");
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);
        
        let shared_indicator_mutex = Arc::new(Mutex::new(false));
        let indicator_clone = shared_indicator_mutex.clone();
                
        let handler = ReceiveMessageHandler { 
            message_received_mutex: indicator_clone
        };

        let running_bot: RunningBot = start_running_bot(handler).await;

        // Wait for other bot to connect and then send message
        let message = MessageBuilder::new()
            .push(RECEIVE_CONTENT)
            .build();

        thread::sleep(Duration::from_secs(2));

        if let Err(_message_sent) = channelid.say(&http, &message).await {
            panic!("test message not sent!");
        }

        // Give time to receive message
        thread::sleep(Duration::from_secs(2));


        // Finally check if MESSAGE_INDICATOR changed (indicating the 2nd bot received the message)
        // And reset to false
        if *shared_indicator_mutex.lock().unwrap() == false
        {
            panic!("test message not received!");
        }

        // Shutdown bot 1
        running_bot.shard_manager.lock().await.shutdown_all().await;

        Ok(())
    }

    #[tokio::test]
    async fn can_embed_a_message() -> Result<(), Error>
    { 
        let token1 = gettoken();
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);
        
        let shared_indicator_mutex: Arc<Mutex<bool>> = Arc::new(Mutex::new(false));
        let indicator_clone = shared_indicator_mutex.clone();

        let handler = ReceiveEmbedMessageHandler{ message_received_mutex: indicator_clone };
        let running_bot: RunningBot = start_running_bot(handler).await;
        
        if let Err(_message_sent) = channelid.send_message(&http, |m| 
        {
            m.content(RECEIVE_EMBED_CONTENT);
            m.embed(|e|
            {
                e.title("Epic Embed");
                e.description("Epic Embed but in smaller font");
                e.fields(vec![("wtf is this", "like wtf is this", true)]);
                e
            });
            m
        }).await {
            panic!("test message not sent!");
        }

        // Give time to receive message
        thread::sleep(Duration::from_secs(2));


        // Finally check if MESSAGE_INDICATOR changed (indicating the 2nd bot received the message)
        // And reset to false
        if *shared_indicator_mutex.lock().unwrap() == false
        {
            panic!("test message not received!");
        }

        // Shutdown bot 1
        running_bot.shard_manager.lock().await.shutdown_all().await;
        Ok(())
    }

    #[tokio::test]
    async fn can_embed_an_image_message() -> Result<(), Error>
    {    
        let token1 = gettoken();
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);
        
        let shared_indicator_mutex: Arc<Mutex<bool>> = Arc::new(Mutex::new(false));
        let indicator_clone = shared_indicator_mutex.clone();

        let handler = ReceiveImageEmbedMessageHandler{ message_received_mutex: indicator_clone };
        let running_bot: RunningBot = start_running_bot(handler).await;
        
        if let Err(_message_sent) = channelid.send_message(&http, |m| 
        {
            m.content(RECEIVE_IMAGE_EMBED_CONTENT);
            m.embed(|e|
            {
                e.title("Epic Embed");
                e.description("Epic Embed but in smaller font");
                e.fields(vec![("wtf is this", "like wtf is this", true)]);
                e.image(TEST_IMAGE);
                e
            });
            m
        }).await {
            panic!("test message not sent!");
        }

        // Give time to receive message
        thread::sleep(Duration::from_secs(2));

        // Finally check if MESSAGE_INDICATOR changed (indicating the 2nd bot received the message)
        // And reset to false
        if *shared_indicator_mutex.lock().unwrap() == false
        {
            panic!("test message not received!");
        }

        // Shutdown bot 1
        running_bot.shard_manager.lock().await.shutdown_all().await;
        Ok(())
    }

    #[tokio::test]
    async fn second_bot_starts_first_bot_when_dead() -> Result<(), Error>
    {
        // something like
        // make sure 1st bot not running
        // start 2nd bot
        // wait a bit
        // check if 1st bot running

        let token = gettoken();
        let handler = StartOtherBotHandler{};

        thread::sleep(Duration::from_secs(2));


        // Check if other bot is running
        // Either use the bot's http to check the bot UserId's status
        // Or ask the bot whether it thinks the bot is online
        

        Ok(())
    }

    #[tokio::test]
    async fn crashing_returns_an_informative_status_code() -> Result<(), Error> {
        let token2 = getwatchertoken();
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token2);
        
        let shared_indicator_mutex: Arc<Mutex<bool>> = Arc::new(Mutex::new(false));
        let indicator_clone = shared_indicator_mutex.clone();

        let handler = ReceiveImageEmbedMessageHandler{ message_received_mutex: indicator_clone };
        let trampoline: RunningBot = start_running_bot(DoNothing{}).await;

        if let Err(_message_sent) = channelid.send_message(&http, |m| 
            {
                m.content(CRASH);
                m
            }).await {
                panic!("test message not sent!");
            }
        //refactor main.rs so that it can accept a switch
        //start a trampoline bot in a separate thread
        //pass it a handler that starts vectorizer with a custom handler
        //make a custom handler that only calls crash

        Ok(())
    }
}